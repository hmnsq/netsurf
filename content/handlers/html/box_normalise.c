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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "css/select.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/table.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_manipulate.h"
#અન્તર્ભાવ "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
સંરચના span_info {
	/** Number of rows this cell spans */
	unsigned પૂર્ણાંક row_span;
	/** Row group of cell */
	સંરચના box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
સંરચના columns {
	/** Current column index */
	unsigned પૂર્ણાંક current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned પૂર્ણાંક અંક_columns;
	/** Information about columns in main table, array [0, num_columns) */
	સંરચના span_info *spans;
	/** Number of rows in table */
	unsigned પૂર્ણાંક અંક_rows;
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
સ્થિર bool
calculate_table_row(સંરચના columns *col_info,
		    unsigned પૂર્ણાંક col_span,
		    unsigned પૂર્ણાંક row_span,
		    unsigned પૂર્ણાંક *start_column,
		    સંરચના box *cell)
{
	unsigned પૂર્ણાંક cell_start_col = col_info->current_column;
	unsigned પૂર્ણાંક cell_end_col;
	unsigned પૂર્ણાંક i;
	સંરચના span_info *spans;
	સંરચના box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	જ્યારે (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	જો (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	જો (col_info->અંક_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				કદ *spans * (cell_end_col + 1));
		જો (spans == ખાલી)
			પાછા false;

		col_info->spans = spans;
		col_info->અંક_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	માટે (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	પાછા true;
}


સ્થિર bool
box_normalise_table_row(સંરચના box *row,
			અચલ સંરચના box *મૂળ,
			સંરચના columns *col_info,
			html_content * c)
{
	સંરચના box *child;
	સંરચના box *next_child;
	સંરચના box *cell = ખાલી;
	css_computed_style *style;
	unsigned પૂર્ણાંક i;
	nscss_select_ctx ctx;

	પ્રતિજ્ઞા(row != ખાલી);
	પ્રતિજ્ઞા(row->type == BOX_TABLE_ROW);

	ctx.મૂળ_style = મૂળ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	માટે (child = row->children; child != ખાલી; child = next_child) {
		next_child = child->next;

		પસંદગી (child->type) {
		સ્થિતિ BOX_TABLE_CELL:
			/* ok */
			જો (box_normalise_block(child, મૂળ, c) == false)
				પાછા false;
			cell = child;
			તોડો;
		સ્થિતિ BOX_FLEX:
		સ્થિતિ BOX_BLOCK:
		સ્થિતિ BOX_INLINE_CONTAINER:
		સ્થિતિ BOX_TABLE:
		સ્થિતિ BOX_TABLE_ROW_GROUP:
		સ્થિતિ BOX_TABLE_ROW:
			/* insert implied table cell */
			પ્રતિજ્ઞા(row->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			જો (style == ખાલી)
				પાછા false;

			cell = box_create(ખાલી, style, true, row->href,
					row->target, ખાલી, ખાલી, c->bctx);
			જો (cell == ખાલી) {
				css_computed_style_destroy(style);
				પાછા false;
			}
			cell->type = BOX_TABLE_CELL;

			જો (child->prev == ખાલી)
				row->children = cell;
			નહીંતર
				child->prev->next = cell;

			cell->prev = child->prev;

			જ્યારે (child != ખાલી && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			પ્રતિજ્ઞા(cell->last != ખાલી);

			cell->last->next = ખાલી;
			cell->next = next_child = child;
			જો (cell->next != ખાલી)
				cell->next->prev = cell;
			નહીંતર
				row->last = cell;
			cell->parent = row;

			જો (box_normalise_block(cell, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_INLINE_FLEX:
		સ્થિતિ BOX_INLINE_BLOCK:
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			પ્રતિજ્ઞા(0);
			તોડો;
		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}

		જો (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			પાછા false;
	}


	/* Update row spanning details for all columns */
	માટે (i = 0; i < col_info->અંક_columns; i++) {
		જો (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->અંક_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	પાછા true;
}


સ્થિર bool
box_normalise_table_row_group(સંરચના box *row_group,
			      અચલ સંરચના box *મૂળ,
			      સંરચના columns *col_info,
			      html_content * c)
{
	સંરચના box *child;
	સંરચના box *next_child;
	સંરચના box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned પૂર્ણાંક group_row_ગણતરી = 0;

	પ્રતિજ્ઞા(row_group != 0);
	પ્રતિજ્ઞા(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.મૂળ_style = મૂળ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	માટે (child = row_group->children; child != ખાલી; child = next_child) {
		next_child = child->next;

		પસંદગી (child->type) {
		સ્થિતિ BOX_TABLE_ROW:
			/* ok */
			group_row_ગણતરી++;
			જો (box_normalise_table_row(child, મૂળ, col_info,
					c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_FLEX:
		સ્થિતિ BOX_BLOCK:
		સ્થિતિ BOX_INLINE_CONTAINER:
		સ્થિતિ BOX_TABLE:
		સ્થિતિ BOX_TABLE_ROW_GROUP:
		સ્થિતિ BOX_TABLE_CELL:
			/* insert implied table row */
			પ્રતિજ્ઞા(row_group->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			જો (style == ખાલી)
				પાછા false;

			row = box_create(ખાલી, style, true, row_group->href,
					row_group->target, ખાલી, ખાલી, c->bctx);
			જો (row == ખાલી) {
				css_computed_style_destroy(style);
				પાછા false;
			}
			row->type = BOX_TABLE_ROW;

			જો (child->prev == ખાલી)
				row_group->children = row;
			નહીંતર
				child->prev->next = row;

			row->prev = child->prev;

			જ્યારે (child != ખાલી && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			પ્રતિજ્ઞા(row->last != ખાલી);

			row->last->next = ખાલી;
			row->next = next_child = child;
			જો (row->next != ખાલી)
				row->next->prev = row;
			નહીંતર
				row_group->last = row;
			row->parent = row_group;

			group_row_ગણતરી++;
			જો (box_normalise_table_row(row, મૂળ, col_info,
					c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_INLINE_FLEX:
		સ્થિતિ BOX_INLINE_BLOCK:
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			પ્રતિજ્ઞા(0);
			તોડો;
		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}
	}

	જો (row_group->children == ખાલી) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		પ્રતિજ્ઞા(row_group->style != ખાલી);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		જો (style == ખાલી) {
			પાછા false;
		}

		row = box_create(ખાલી, style, true, row_group->href,
				row_group->target, ખાલી, ખાલી, c->bctx);
		જો (row == ખાલી) {
			css_computed_style_destroy(style);
			પાછા false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_ગણતરી = 1;

		/* Keep table's row count in sync */
		col_info->અંક_rows++;
	}

	row_group->rows = group_row_ગણતરી;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	પાછા true;
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
સ્થિર bool
box_normalise_table_spans(સંરચના box *table,
			  અચલ સંરચના box *મૂળ,
			  સંરચના span_info *spans,
			  html_content *c)
{
	સંરચના box *table_row_group;
	સંરચના box *table_row;
	સંરચના box *table_cell;
	unsigned પૂર્ણાંક rows_ડાબું = table->rows;
	unsigned પૂર્ણાંક group_rows_ડાબું;
	unsigned પૂર્ણાંક col;
	nscss_select_ctx ctx;

	ctx.મૂળ_style = મૂળ->style;

	/* Clear span data */
	memset(spans, 0, table->columns * કદ(સંરચના span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	માટે (table_row_group = table->children;
	     table_row_group != ખાલી;
	     table_row_group = table_row_group->next) {

		group_rows_ડાબું = table_row_group->rows;

		માટે (table_row = table_row_group->children;
		     table_row != ખાલી;
		     table_row = table_row->next) {

			માટે (table_cell = table_row->children;
			     table_cell != ખાલી;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				જો (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				જો (table_cell->rows == 0) {
					table_cell->rows = group_rows_ડાબું;
				}

				/* limit rowspans within group */
				જો (table_cell->rows > group_rows_ડાબું) {
					table_cell->rows = group_rows_ડાબું;
				}

				/* Record span information */
				માટે (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			માટે (col = 0; col < table->columns; col++) {
				જો (spans[col].row_span == 0) {
					unsigned પૂર્ણાંક start = col;
					css_computed_style *style;
					સંરચના box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					પ્રતિજ્ઞા(table_row->style != ખાલી);

					/* Find width of gap */
					જ્યારે (col < table->columns &&
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
					જો (style == ખાલી)
						પાછા false;

					cell = box_create(ખાલી, style, true,
							table_row->href,
							table_row->target,
							ખાલી, ખાલી, c->bctx);
					જો (cell == ખાલી) {
						css_computed_style_destroy(
								style);
						પાછા false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					માટે (prev = table_row->children;
							prev != ખાલી;
							prev = prev->next) {
						જો (prev->start_column +
							prev->columns ==
								start)
							તોડો;
						જો (prev->next == ખાલી)
							તોડો;
					}

					/* Insert it */
					જો (prev == ખાલી) {
						જો (table_row->children != ખાલી)
							table_row->children->
								prev = cell;
						નહીંતર
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} નહીંતર {
						જો (prev->next != ખાલી)
							prev->next->prev = cell;
						નહીંતર
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} નહીંતર {
					spans[col].row_span--;
				}
			}

			પ્રતિજ્ઞા(rows_ડાબું > 0);

			rows_ડાબું--;
		}

		group_rows_ડાબું--;
	}

	પાછા true;
}


સ્થિર bool
box_normalise_table(સંરચના box *table, અચલ સંરચના box *મૂળ, html_content * c)
{
	સંરચના box *child;
	સંરચના box *next_child;
	સંરચના box *row_group;
	css_computed_style *style;
	સંરચના columns col_info;
	nscss_select_ctx ctx;

	પ્રતિજ્ઞા(table != ખાલી);
	પ્રતિજ્ઞા(table->type == BOX_TABLE);

	ctx.મૂળ_style = મૂળ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.અંક_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * કદ *col_info.spans);
	જો (col_info.spans == ખાલી)
		પાછા false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.અંક_rows = 0;

	માટે (child = table->children; child != ખાલી; child = next_child) {
		next_child = child->next;
		પસંદગી (child->type) {
		સ્થિતિ BOX_TABLE_ROW_GROUP:
			/* ok */
			જો (box_normalise_table_row_group(child, મૂળ,
					&col_info, c) == false) {
				મુક્ત(col_info.spans);
				પાછા false;
			}
			તોડો;
		સ્થિતિ BOX_FLEX:
		સ્થિતિ BOX_BLOCK:
		સ્થિતિ BOX_INLINE_CONTAINER:
		સ્થિતિ BOX_TABLE:
		સ્થિતિ BOX_TABLE_ROW:
		સ્થિતિ BOX_TABLE_CELL:
			/* insert implied table row group */
			પ્રતિજ્ઞા(table->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			જો (style == ખાલી) {
				મુક્ત(col_info.spans);
				પાછા false;
			}

			row_group = box_create(ખાલી, style, true, table->href,
					table->target, ખાલી, ખાલી, c->bctx);
			જો (row_group == ખાલી) {
				css_computed_style_destroy(style);
				મુક્ત(col_info.spans);
				પાછા false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			જો (child->prev == ખાલી)
				table->children = row_group;
			નહીંતર
				child->prev->next = row_group;

			row_group->prev = child->prev;

			જ્યારે (child != ખાલી && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			પ્રતિજ્ઞા(row_group->last != ખાલી);

			row_group->last->next = ખાલી;
			row_group->next = next_child = child;
			જો (row_group->next != ખાલી)
				row_group->next->prev = row_group;
			નહીંતર
				table->last = row_group;
			row_group->parent = table;

			જો (box_normalise_table_row_group(row_group, મૂળ,
					&col_info, c) == false) {
				મુક્ત(col_info.spans);
				પાછા false;
			}
			તોડો;
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_INLINE_FLEX:
		સ્થિતિ BOX_INLINE_BLOCK:
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			પ્રતિજ્ઞા(0);
			તોડો;
		મૂળભૂત:
			fprintf(stderr, "%i\n", child->type);
			પ્રતિજ્ઞા(0);
		}
	}

	table->columns = col_info.અંક_columns;
	table->rows = col_info.અંક_rows;

	જો (table->children == ખાલી) {
		સંરચના box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		પ્રતિજ્ઞા(table->style != ખાલી);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		જો (style == ખાલી) {
			મુક્ત(col_info.spans);
			પાછા false;
		}

		row_group = box_create(ખાલી, style, true, table->href,
				table->target, ખાલી, ખાલી, c->bctx);
		જો (row_group == ખાલી) {
			css_computed_style_destroy(style);
			મુક્ત(col_info.spans);
			પાછા false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		જો (style == ખાલી) {
			box_free(row_group);
			મુક્ત(col_info.spans);
			પાછા false;
		}

		row = box_create(ખાલી, style, true, row_group->href,
				row_group->target, ખાલી, ખાલી, c->bctx);
		જો (row == ખાલી) {
			css_computed_style_destroy(style);
			box_free(row_group);
			મુક્ત(col_info.spans);
			પાછા false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	જો (box_normalise_table_spans(table, મૂળ, col_info.spans, c) == false) {
		મુક્ત(col_info.spans);
		પાછા false;
	}

	મુક્ત(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	પાછા true;
}

સ્થિર bool box_normalise_flex(
		સંરચના box *flex_container,
		અચલ સંરચના box *મૂળ,
		html_content *c)
{
	સંરચના box *child;
	સંરચના box *next_child;
	સંરચના box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	પ્રતિજ્ઞા(flex_container != ખાલી);
	પ્રતિજ્ઞા(મૂળ != ખાલી);

	ctx.મૂળ_style = મૂળ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	પ્રતિજ્ઞા(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	માટે (child = flex_container->children; child != ખાલી; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		પસંદગી (child->type) {
		સ્થિતિ BOX_FLEX:
			/* ok */
			જો (box_normalise_flex(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_BLOCK:
			/* ok */
			જો (box_normalise_block(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			પ્રતિજ્ઞા(flex_container->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			જો (style == ખાલી)
				પાછા false;

			implied_flex_item = box_create(ખાલી, style, true,
					flex_container->href,
					flex_container->target,
					ખાલી, ખાલી, c->bctx);
			જો (implied_flex_item == ખાલી) {
				css_computed_style_destroy(style);
				પાછા false;
			}
			implied_flex_item->type = BOX_BLOCK;

			જો (child->prev == ખાલી)
				flex_container->children = implied_flex_item;
			નહીંતર
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			જ્યારે (child != ખાલી &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			implied_flex_item->last->next = ખાલી;
			implied_flex_item->next = next_child = child;
			જો (implied_flex_item->next != ખાલી)
				implied_flex_item->next->prev = implied_flex_item;
			નહીંતર
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			જો (box_normalise_block(implied_flex_item,
					મૂળ, c) == false)
				પાછા false;
			તોડો;

		સ્થિતિ BOX_TABLE:
			જો (box_normalise_table(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_INLINE_FLEX:
		સ્થિતિ BOX_INLINE_BLOCK:
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			પ્રતિજ્ઞા(0);
			તોડો;
		સ્થિતિ BOX_TABLE_ROW_GROUP:
		સ્થિતિ BOX_TABLE_ROW:
		સ્થિતિ BOX_TABLE_CELL:
			/* insert implied table */
			પ્રતિજ્ઞા(flex_container->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			જો (style == ખાલી)
				પાછા false;

			implied_flex_item = box_create(ખાલી, style, true,
					flex_container->href,
					flex_container->target,
					ખાલી, ખાલી, c->bctx);
			જો (implied_flex_item == ખાલી) {
				css_computed_style_destroy(style);
				પાછા false;
			}
			implied_flex_item->type = BOX_TABLE;

			જો (child->prev == ખાલી)
				flex_container->children = implied_flex_item;
			નહીંતર
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			જ્યારે (child != ખાલી && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			implied_flex_item->last->next = ખાલી;
			implied_flex_item->next = next_child = child;
			જો (implied_flex_item->next != ખાલી)
				implied_flex_item->next->prev = implied_flex_item;
			નહીંતર
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			જો (box_normalise_table(implied_flex_item,
					મૂળ, c) == false)
				પાછા false;
			તોડો;
		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}
	}

	પાછા true;
}

સ્થિર bool
box_normalise_inline_container(સંરચના box *cont,
			       અચલ સંરચના box *મૂળ,
			       html_content * c)
{
	સંરચના box *child;
	સંરચના box *next_child;

	પ્રતિજ્ઞા(cont != ખાલી);
	પ્રતિજ્ઞા(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	માટે (child = cont->children; child != ખાલી; child = next_child) {
		next_child = child->next;
		પસંદગી (child->type) {
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* ok */
			તોડો;
		સ્થિતિ BOX_INLINE_BLOCK:
			/* ok */
			જો (box_normalise_block(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE_FLEX:
			/* ok */
			જો (box_normalise_flex(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
			/* ok */
			પ્રતિજ્ઞા(child->children != ખાલી);

			પસંદગી (child->children->type) {
			સ્થિતિ BOX_BLOCK:
				જો (box_normalise_block(child->children, મૂળ,
						c) == false)
					પાછા false;
				તોડો;
			સ્થિતિ BOX_TABLE:
				જો (box_normalise_table(child->children, મૂળ,
						c) == false)
					પાછા false;
				તોડો;
			સ્થિતિ BOX_FLEX:
				જો (box_normalise_flex(child->children, મૂળ,
						c) == false)
					પાછા false;
				તોડો;
			મૂળભૂત:
				પ્રતિજ્ઞા(0);
			}

			જો (child->children == ખાલી) {
				/* the child has destroyed itself: remove float */
				જો (child->prev == ખાલી)
					child->parent->children = child->next;
				નહીંતર
					child->prev->next = child->next;
				જો (child->next != ખાલી)
					child->next->prev = child->prev;
				નહીંતર
					child->parent->last = child->prev;

				box_free(child);
			}
			તોડો;
		સ્થિતિ BOX_FLEX:
		સ્થિતિ BOX_BLOCK:
		સ્થિતિ BOX_INLINE_CONTAINER:
		સ્થિતિ BOX_TABLE:
		સ્થિતિ BOX_TABLE_ROW_GROUP:
		સ્થિતિ BOX_TABLE_ROW:
		સ્થિતિ BOX_TABLE_CELL:
		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	પાછા true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(સંરચના box *block, અચલ સંરચના box *મૂળ, html_content *c)
{
	સંરચના box *child;
	સંરચના box *next_child;
	સંરચના box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	પ્રતિજ્ઞા(block != ખાલી);
	પ્રતિજ્ઞા(મૂળ != ખાલી);

	ctx.મૂળ_style = મૂળ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	પ્રતિજ્ઞા(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	માટે (child = block->children; child != ખાલી; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		પસંદગી (child->type) {
		સ્થિતિ BOX_FLEX:
			/* ok */
			જો (box_normalise_flex(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_BLOCK:
			/* ok */
			જો (box_normalise_block(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE_CONTAINER:
			જો (box_normalise_inline_container(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_TABLE:
			જો (box_normalise_table(child, મૂળ, c) == false)
				પાછા false;
			તોડો;
		સ્થિતિ BOX_INLINE:
		સ્થિતિ BOX_INLINE_END:
		સ્થિતિ BOX_INLINE_FLEX:
		સ્થિતિ BOX_INLINE_BLOCK:
		સ્થિતિ BOX_FLOAT_ડાબું:
		સ્થિતિ BOX_FLOAT_જમણું:
		સ્થિતિ BOX_BR:
		સ્થિતિ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			પ્રતિજ્ઞા(0);
			તોડો;
		સ્થિતિ BOX_TABLE_ROW_GROUP:
		સ્થિતિ BOX_TABLE_ROW:
		સ્થિતિ BOX_TABLE_CELL:
			/* insert implied table */
			પ્રતિજ્ઞા(block->style != ખાલી);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			જો (style == ખાલી)
				પાછા false;

			table = box_create(ખાલી, style, true, block->href,
					block->target, ખાલી, ખાલી, c->bctx);
			જો (table == ખાલી) {
				css_computed_style_destroy(style);
				પાછા false;
			}
			table->type = BOX_TABLE;

			જો (child->prev == ખાલી)
				block->children = table;
			નહીંતર
				child->prev->next = table;

			table->prev = child->prev;

			જ્યારે (child != ખાલી && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = ખાલી;
				child = next_child;
			}

			table->last->next = ખાલી;
			table->next = next_child = child;
			જો (table->next != ખાલી)
				table->next->prev = table;
			નહીંતર
				block->last = table;
			table->parent = block;

			જો (box_normalise_table(table, મૂળ, c) == false)
				પાછા false;
			તોડો;
		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}
	}

	પાછા true;
}
