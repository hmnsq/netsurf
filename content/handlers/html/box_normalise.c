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
	unsigned पूर्णाङ्क row_span;
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
	unsigned पूर्णाङ्क current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned पूर्णाङ्क अंक_columns;
	/** Information about columns in main table, array [0, num_columns) */
	संरचना span_info *spans;
	/** Number of rows in table */
	unsigned पूर्णाङ्क अंक_rows;
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
		    unsigned पूर्णाङ्क col_span,
		    unsigned पूर्णाङ्क row_span,
		    unsigned पूर्णाङ्क *start_column,
		    संरचना box *cell)
{
	unsigned पूर्णाङ्क cell_start_col = col_info->current_column;
	unsigned पूर्णाङ्क cell_end_col;
	unsigned पूर्णाङ्क i;
	संरचना span_info *spans;
	संरचना box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	यावत् (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	यदि (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	यदि (col_info->अंक_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				परिमाण *spans * (cell_end_col + 1));
		यदि (spans == रिक्त)
			प्रत्यागम false;

		col_info->spans = spans;
		col_info->अंक_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	प्रति (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	प्रत्यागम true;
}


स्थिर bool
box_normalise_table_row(संरचना box *row,
			ध्रुव संरचना box *मूल,
			संरचना columns *col_info,
			html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *cell = रिक्त;
	css_computed_style *style;
	unsigned पूर्णाङ्क i;
	nscss_select_ctx ctx;

	प्रतिज्ञा(row != रिक्त);
	प्रतिज्ञा(row->type == BOX_TABLE_ROW);

	ctx.मूल_style = मूल->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	प्रति (child = row->children; child != रिक्त; child = next_child) {
		next_child = child->next;

		विकल्प (child->type) {
		पक्ष BOX_TABLE_CELL:
			/* ok */
			यदि (box_normalise_block(child, मूल, c) == false)
				प्रत्यागम false;
			cell = child;
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_BLOCK:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_TABLE:
		पक्ष BOX_TABLE_ROW_GROUP:
		पक्ष BOX_TABLE_ROW:
			/* insert implied table cell */
			प्रतिज्ञा(row->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			cell = box_create(रिक्त, style, true, row->href,
					row->target, रिक्त, रिक्त, c->bctx);
			यदि (cell == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			cell->type = BOX_TABLE_CELL;

			यदि (child->prev == रिक्त)
				row->children = cell;
			अन्यथा
				child->prev->next = cell;

			cell->prev = child->prev;

			यावत् (child != रिक्त && (
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
			यदि (cell->next != रिक्त)
				cell->next->prev = cell;
			अन्यथा
				row->last = cell;
			cell->parent = row;

			यदि (box_normalise_block(cell, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_BLOCK:
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}

		यदि (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			प्रत्यागम false;
	}


	/* Update row spanning details for all columns */
	प्रति (i = 0; i < col_info->अंक_columns; i++) {
		यदि (col_info->spans[i].row_span != 0 &&
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

	प्रत्यागम true;
}


स्थिर bool
box_normalise_table_row_group(संरचना box *row_group,
			      ध्रुव संरचना box *मूल,
			      संरचना columns *col_info,
			      html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned पूर्णाङ्क group_row_गिनती = 0;

	प्रतिज्ञा(row_group != 0);
	प्रतिज्ञा(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.मूल_style = मूल->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	प्रति (child = row_group->children; child != रिक्त; child = next_child) {
		next_child = child->next;

		विकल्प (child->type) {
		पक्ष BOX_TABLE_ROW:
			/* ok */
			group_row_गिनती++;
			यदि (box_normalise_table_row(child, मूल, col_info,
					c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_BLOCK:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_TABLE:
		पक्ष BOX_TABLE_ROW_GROUP:
		पक्ष BOX_TABLE_CELL:
			/* insert implied table row */
			प्रतिज्ञा(row_group->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			row = box_create(रिक्त, style, true, row_group->href,
					row_group->target, रिक्त, रिक्त, c->bctx);
			यदि (row == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			row->type = BOX_TABLE_ROW;

			यदि (child->prev == रिक्त)
				row_group->children = row;
			अन्यथा
				child->prev->next = row;

			row->prev = child->prev;

			यावत् (child != रिक्त && (
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
			यदि (row->next != रिक्त)
				row->next->prev = row;
			अन्यथा
				row_group->last = row;
			row->parent = row_group;

			group_row_गिनती++;
			यदि (box_normalise_table_row(row, मूल, col_info,
					c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_BLOCK:
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

	यदि (row_group->children == रिक्त) {
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
		यदि (style == रिक्त) {
			प्रत्यागम false;
		}

		row = box_create(रिक्त, style, true, row_group->href,
				row_group->target, रिक्त, रिक्त, c->bctx);
		यदि (row == रिक्त) {
			css_computed_style_destroy(style);
			प्रत्यागम false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_गिनती = 1;

		/* Keep table's row count in sync */
		col_info->अंक_rows++;
	}

	row_group->rows = group_row_गिनती;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	प्रत्यागम true;
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
			  ध्रुव संरचना box *मूल,
			  संरचना span_info *spans,
			  html_content *c)
{
	संरचना box *table_row_group;
	संरचना box *table_row;
	संरचना box *table_cell;
	unsigned पूर्णाङ्क rows_बायाँ = table->rows;
	unsigned पूर्णाङ्क group_rows_बायाँ;
	unsigned पूर्णाङ्क col;
	nscss_select_ctx ctx;

	ctx.मूल_style = मूल->style;

	/* Clear span data */
	memset(spans, 0, table->columns * परिमाण(संरचना span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	प्रति (table_row_group = table->children;
	     table_row_group != रिक्त;
	     table_row_group = table_row_group->next) {

		group_rows_बायाँ = table_row_group->rows;

		प्रति (table_row = table_row_group->children;
		     table_row != रिक्त;
		     table_row = table_row->next) {

			प्रति (table_cell = table_row->children;
			     table_cell != रिक्त;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				यदि (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				यदि (table_cell->rows == 0) {
					table_cell->rows = group_rows_बायाँ;
				}

				/* limit rowspans within group */
				यदि (table_cell->rows > group_rows_बायाँ) {
					table_cell->rows = group_rows_बायाँ;
				}

				/* Record span information */
				प्रति (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			प्रति (col = 0; col < table->columns; col++) {
				यदि (spans[col].row_span == 0) {
					unsigned पूर्णाङ्क start = col;
					css_computed_style *style;
					संरचना box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					प्रतिज्ञा(table_row->style != रिक्त);

					/* Find width of gap */
					यावत् (col < table->columns &&
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
					यदि (style == रिक्त)
						प्रत्यागम false;

					cell = box_create(रिक्त, style, true,
							table_row->href,
							table_row->target,
							रिक्त, रिक्त, c->bctx);
					यदि (cell == रिक्त) {
						css_computed_style_destroy(
								style);
						प्रत्यागम false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					प्रति (prev = table_row->children;
							prev != रिक्त;
							prev = prev->next) {
						यदि (prev->start_column +
							prev->columns ==
								start)
							भङ्ग;
						यदि (prev->next == रिक्त)
							भङ्ग;
					}

					/* Insert it */
					यदि (prev == रिक्त) {
						यदि (table_row->children != रिक्त)
							table_row->children->
								prev = cell;
						अन्यथा
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} अन्यथा {
						यदि (prev->next != रिक्त)
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

			प्रतिज्ञा(rows_बायाँ > 0);

			rows_बायाँ--;
		}

		group_rows_बायाँ--;
	}

	प्रत्यागम true;
}


स्थिर bool
box_normalise_table(संरचना box *table, ध्रुव संरचना box *मूल, html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *row_group;
	css_computed_style *style;
	संरचना columns col_info;
	nscss_select_ctx ctx;

	प्रतिज्ञा(table != रिक्त);
	प्रतिज्ञा(table->type == BOX_TABLE);

	ctx.मूल_style = मूल->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.अंक_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * परिमाण *col_info.spans);
	यदि (col_info.spans == रिक्त)
		प्रत्यागम false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.अंक_rows = 0;

	प्रति (child = table->children; child != रिक्त; child = next_child) {
		next_child = child->next;
		विकल्प (child->type) {
		पक्ष BOX_TABLE_ROW_GROUP:
			/* ok */
			यदि (box_normalise_table_row_group(child, मूल,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_BLOCK:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_TABLE:
		पक्ष BOX_TABLE_ROW:
		पक्ष BOX_TABLE_CELL:
			/* insert implied table row group */
			प्रतिज्ञा(table->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			यदि (style == रिक्त) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}

			row_group = box_create(रिक्त, style, true, table->href,
					table->target, रिक्त, रिक्त, c->bctx);
			यदि (row_group == रिक्त) {
				css_computed_style_destroy(style);
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			यदि (child->prev == रिक्त)
				table->children = row_group;
			अन्यथा
				child->prev->next = row_group;

			row_group->prev = child->prev;

			यावत् (child != रिक्त && (
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
			यदि (row_group->next != रिक्त)
				row_group->next->prev = row_group;
			अन्यथा
				table->last = row_group;
			row_group->parent = table;

			यदि (box_normalise_table_row_group(row_group, मूल,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_BLOCK:
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		स्वभाव:
			fprintf(stderr, "%i\n", child->type);
			प्रतिज्ञा(0);
		}
	}

	table->columns = col_info.अंक_columns;
	table->rows = col_info.अंक_rows;

	यदि (table->children == रिक्त) {
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
		यदि (style == रिक्त) {
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}

		row_group = box_create(रिक्त, style, true, table->href,
				table->target, रिक्त, रिक्त, c->bctx);
		यदि (row_group == रिक्त) {
			css_computed_style_destroy(style);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		यदि (style == रिक्त) {
			box_free(row_group);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}

		row = box_create(रिक्त, style, true, row_group->href,
				row_group->target, रिक्त, रिक्त, c->bctx);
		यदि (row == रिक्त) {
			css_computed_style_destroy(style);
			box_free(row_group);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	यदि (box_normalise_table_spans(table, मूल, col_info.spans, c) == false) {
		मुक्त(col_info.spans);
		प्रत्यागम false;
	}

	मुक्त(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	प्रत्यागम true;
}

स्थिर bool box_normalise_flex(
		संरचना box *flex_container,
		ध्रुव संरचना box *मूल,
		html_content *c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(flex_container != रिक्त);
	प्रतिज्ञा(मूल != रिक्त);

	ctx.मूल_style = मूल->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	प्रतिज्ञा(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	प्रति (child = flex_container->children; child != रिक्त; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		विकल्प (child->type) {
		पक्ष BOX_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_BLOCK:
			/* ok */
			यदि (box_normalise_block(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			implied_flex_item = box_create(रिक्त, style, true,
					flex_container->href,
					flex_container->target,
					रिक्त, रिक्त, c->bctx);
			यदि (implied_flex_item == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			implied_flex_item->type = BOX_BLOCK;

			यदि (child->prev == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			यावत् (child != रिक्त &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			implied_flex_item->last->next = रिक्त;
			implied_flex_item->next = next_child = child;
			यदि (implied_flex_item->next != रिक्त)
				implied_flex_item->next->prev = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			यदि (box_normalise_block(implied_flex_item,
					मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;

		पक्ष BOX_TABLE:
			यदि (box_normalise_table(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_BLOCK:
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		पक्ष BOX_TABLE_ROW_GROUP:
		पक्ष BOX_TABLE_ROW:
		पक्ष BOX_TABLE_CELL:
			/* insert implied table */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			implied_flex_item = box_create(रिक्त, style, true,
					flex_container->href,
					flex_container->target,
					रिक्त, रिक्त, c->bctx);
			यदि (implied_flex_item == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			implied_flex_item->type = BOX_TABLE;

			यदि (child->prev == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			यावत् (child != रिक्त && (
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
			यदि (implied_flex_item->next != रिक्त)
				implied_flex_item->next->prev = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			यदि (box_normalise_table(implied_flex_item,
					मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

	प्रत्यागम true;
}

स्थिर bool
box_normalise_inline_container(संरचना box *cont,
			       ध्रुव संरचना box *मूल,
			       html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;

	प्रतिज्ञा(cont != रिक्त);
	प्रतिज्ञा(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	प्रति (child = cont->children; child != रिक्त; child = next_child) {
		next_child = child->next;
		विकल्प (child->type) {
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* ok */
			भङ्ग;
		पक्ष BOX_INLINE_BLOCK:
			/* ok */
			यदि (box_normalise_block(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
			/* ok */
			प्रतिज्ञा(child->children != रिक्त);

			विकल्प (child->children->type) {
			पक्ष BOX_BLOCK:
				यदि (box_normalise_block(child->children, मूल,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			पक्ष BOX_TABLE:
				यदि (box_normalise_table(child->children, मूल,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			पक्ष BOX_FLEX:
				यदि (box_normalise_flex(child->children, मूल,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			स्वभाव:
				प्रतिज्ञा(0);
			}

			यदि (child->children == रिक्त) {
				/* the child has destroyed itself: remove float */
				यदि (child->prev == रिक्त)
					child->parent->children = child->next;
				अन्यथा
					child->prev->next = child->next;
				यदि (child->next != रिक्त)
					child->next->prev = child->prev;
				अन्यथा
					child->parent->last = child->prev;

				box_free(child);
			}
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_BLOCK:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_TABLE:
		पक्ष BOX_TABLE_ROW_GROUP:
		पक्ष BOX_TABLE_ROW:
		पक्ष BOX_TABLE_CELL:
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	प्रत्यागम true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(संरचना box *block, ध्रुव संरचना box *मूल, html_content *c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(block != रिक्त);
	प्रतिज्ञा(मूल != रिक्त);

	ctx.मूल_style = मूल->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	प्रतिज्ञा(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	प्रति (child = block->children; child != रिक्त; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		विकल्प (child->type) {
		पक्ष BOX_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_BLOCK:
			/* ok */
			यदि (box_normalise_block(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_CONTAINER:
			यदि (box_normalise_inline_container(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_TABLE:
			यदि (box_normalise_table(child, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_END:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_BLOCK:
		पक्ष BOX_FLOAT_बायाँ:
		पक्ष BOX_FLOAT_दायाँ:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		पक्ष BOX_TABLE_ROW_GROUP:
		पक्ष BOX_TABLE_ROW:
		पक्ष BOX_TABLE_CELL:
			/* insert implied table */
			प्रतिज्ञा(block->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			table = box_create(रिक्त, style, true, block->href,
					block->target, रिक्त, रिक्त, c->bctx);
			यदि (table == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			table->type = BOX_TABLE;

			यदि (child->prev == रिक्त)
				block->children = table;
			अन्यथा
				child->prev->next = table;

			table->prev = child->prev;

			यावत् (child != रिक्त && (
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
			यदि (table->next != रिक्त)
				table->next->prev = table;
			अन्यथा
				block->last = table;
			table->parent = block;

			यदि (box_normalise_table(table, मूल, c) == false)
				प्रत्यागम false;
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

	प्रत्यागम true;
}
