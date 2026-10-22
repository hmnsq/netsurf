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

#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <string.h>

#समावेश "utils/log.h"
#समावेश "utils/errors.h"
#समावेश "css/select.h"

#समावेश "html/private.h"
#समावेश "html/table.h"
#समावेश "html/box.h"
#समावेश "html/box_manipulate.h"
#समावेश "html/box_normalise.h"

/* Define to enable box normalise debug */
#अघोषित BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
काष्ठा span_info {
	/** Number of rows this cell spans */
	अचिन्हित पूर्णांक row_span;
	/** Row group of cell */
	काष्ठा box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
काष्ठा columns {
	/** Current column index */
	अचिन्हित पूर्णांक current_column;
	/** Number of columns in main part of table 1..max columns */
	अचिन्हित पूर्णांक अंक_columns;
	/** Information about columns in main table, array [0, num_columns) */
	काष्ठा span_info *spans;
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
अटल bool
calculate_table_row(काष्ठा columns *col_info,
		    अचिन्हित पूर्णांक col_span,
		    अचिन्हित पूर्णांक row_span,
		    अचिन्हित पूर्णांक *start_column,
		    काष्ठा box *cell)
{
	अचिन्हित पूर्णांक cell_start_col = col_info->current_column;
	अचिन्हित पूर्णांक cell_end_col;
	अचिन्हित पूर्णांक i;
	काष्ठा span_info *spans;
	काष्ठा box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	जबतक (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	अगर (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	अगर (col_info->अंक_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = पुनः_स्मृति(col_info->spans,
				माप *spans * (cell_end_col + 1));
		अगर (spans == शून्य)
			वापस false;

		col_info->spans = spans;
		col_info->अंक_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	क्रम (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	वापस true;
}


अटल bool
box_normalise_table_row(काष्ठा box *row,
			स्थिर काष्ठा box *मूल,
			काष्ठा columns *col_info,
			html_content * c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;
	काष्ठा box *cell = शून्य;
	css_computed_style *style;
	अचिन्हित पूर्णांक i;
	nscss_select_ctx ctx;

	निश्चित(row != शून्य);
	निश्चित(row->type == BOX_TABLE_ROW);

	ctx.मूल_style = मूल->style;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#पूर्ण_अगर

	क्रम (child = row->children; child != शून्य; child = अग्र_child) {
		अग्र_child = child->अग्र;

		चयन (child->type) {
		हाल BOX_TABLE_CELL:
			/* ok */
			अगर (box_normalise_block(child, मूल, c) == false)
				वापस false;
			cell = child;
			अवरोध;
		हाल BOX_FLEX:
		हाल BOX_BLOCK:
		हाल BOX_INLINE_CONTAINER:
		हाल BOX_TABLE:
		हाल BOX_TABLE_ROW_GROUP:
		हाल BOX_TABLE_ROW:
			/* insert implied table cell */
			निश्चित(row->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			अगर (style == शून्य)
				वापस false;

			cell = box_create(शून्य, style, true, row->href,
					row->target, शून्य, शून्य, c->bctx);
			अगर (cell == शून्य) {
				css_computed_style_destroy(style);
				वापस false;
			}
			cell->type = BOX_TABLE_CELL;

			अगर (child->पूर्व == शून्य)
				row->children = cell;
			अन्यथा
				child->पूर्व->अग्र = cell;

			cell->पूर्व = child->पूर्व;

			जबतक (child != शून्य && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			निश्चित(cell->last != शून्य);

			cell->last->अग्र = शून्य;
			cell->अग्र = अग्र_child = child;
			अगर (cell->अग्र != शून्य)
				cell->अग्र->पूर्व = cell;
			अन्यथा
				row->last = cell;
			cell->parent = row;

			अगर (box_normalise_block(cell, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_INLINE_FLEX:
		हाल BOX_INLINE_BLOCK:
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			निश्चित(0);
			अवरोध;
		शेष:
			निश्चित(0);
		}

		अगर (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			वापस false;
	}


	/* Update row spanning details for all columns */
	क्रम (i = 0; i < col_info->अंक_columns; i++) {
		अगर (col_info->spans[i].row_span != 0 &&
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

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#पूर्ण_अगर

	वापस true;
}


अटल bool
box_normalise_table_row_group(काष्ठा box *row_group,
			      स्थिर काष्ठा box *मूल,
			      काष्ठा columns *col_info,
			      html_content * c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;
	काष्ठा box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	अचिन्हित पूर्णांक group_row_गिनती = 0;

	निश्चित(row_group != 0);
	निश्चित(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.मूल_style = मूल->style;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#पूर्ण_अगर

	क्रम (child = row_group->children; child != शून्य; child = अग्र_child) {
		अग्र_child = child->अग्र;

		चयन (child->type) {
		हाल BOX_TABLE_ROW:
			/* ok */
			group_row_गिनती++;
			अगर (box_normalise_table_row(child, मूल, col_info,
					c) == false)
				वापस false;
			अवरोध;
		हाल BOX_FLEX:
		हाल BOX_BLOCK:
		हाल BOX_INLINE_CONTAINER:
		हाल BOX_TABLE:
		हाल BOX_TABLE_ROW_GROUP:
		हाल BOX_TABLE_CELL:
			/* insert implied table row */
			निश्चित(row_group->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			अगर (style == शून्य)
				वापस false;

			row = box_create(शून्य, style, true, row_group->href,
					row_group->target, शून्य, शून्य, c->bctx);
			अगर (row == शून्य) {
				css_computed_style_destroy(style);
				वापस false;
			}
			row->type = BOX_TABLE_ROW;

			अगर (child->पूर्व == शून्य)
				row_group->children = row;
			अन्यथा
				child->पूर्व->अग्र = row;

			row->पूर्व = child->पूर्व;

			जबतक (child != शून्य && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			निश्चित(row->last != शून्य);

			row->last->अग्र = शून्य;
			row->अग्र = अग्र_child = child;
			अगर (row->अग्र != शून्य)
				row->अग्र->पूर्व = row;
			अन्यथा
				row_group->last = row;
			row->parent = row_group;

			group_row_गिनती++;
			अगर (box_normalise_table_row(row, मूल, col_info,
					c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_INLINE_FLEX:
		हाल BOX_INLINE_BLOCK:
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			निश्चित(0);
			अवरोध;
		शेष:
			निश्चित(0);
		}
	}

	अगर (row_group->children == शून्य) {
#अगर_घोषित BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#पूर्ण_अगर

		निश्चित(row_group->style != शून्य);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		अगर (style == शून्य) {
			वापस false;
		}

		row = box_create(शून्य, style, true, row_group->href,
				row_group->target, शून्य, शून्य, c->bctx);
		अगर (row == शून्य) {
			css_computed_style_destroy(style);
			वापस false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_गिनती = 1;

		/* Keep table's row count in sync */
		col_info->अंक_rows++;
	}

	row_group->rows = group_row_गिनती;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#पूर्ण_अगर

	वापस true;
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
अटल bool
box_normalise_table_spans(काष्ठा box *table,
			  स्थिर काष्ठा box *मूल,
			  काष्ठा span_info *spans,
			  html_content *c)
{
	काष्ठा box *table_row_group;
	काष्ठा box *table_row;
	काष्ठा box *table_cell;
	अचिन्हित पूर्णांक rows_बायाँ = table->rows;
	अचिन्हित पूर्णांक group_rows_बायाँ;
	अचिन्हित पूर्णांक col;
	nscss_select_ctx ctx;

	ctx.मूल_style = मूल->style;

	/* Clear span data */
	स_रखो(spans, 0, table->columns * माप(काष्ठा span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	क्रम (table_row_group = table->children;
	     table_row_group != शून्य;
	     table_row_group = table_row_group->अग्र) {

		group_rows_बायाँ = table_row_group->rows;

		क्रम (table_row = table_row_group->children;
		     table_row != शून्य;
		     table_row = table_row->अग्र) {

			क्रम (table_cell = table_row->children;
			     table_cell != शून्य;
			     table_cell = table_cell->अग्र) {

				/* colspan = 0 -> colspan = 1 */
				अगर (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				अगर (table_cell->rows == 0) {
					table_cell->rows = group_rows_बायाँ;
				}

				/* limit rowspans within group */
				अगर (table_cell->rows > group_rows_बायाँ) {
					table_cell->rows = group_rows_बायाँ;
				}

				/* Record span information */
				क्रम (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			क्रम (col = 0; col < table->columns; col++) {
				अगर (spans[col].row_span == 0) {
					अचिन्हित पूर्णांक start = col;
					css_computed_style *style;
					काष्ठा box *cell, *पूर्व;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					निश्चित(table_row->style != शून्य);

					/* Find width of gap */
					जबतक (col < table->columns &&
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
					अगर (style == शून्य)
						वापस false;

					cell = box_create(शून्य, style, true,
							table_row->href,
							table_row->target,
							शून्य, शून्य, c->bctx);
					अगर (cell == शून्य) {
						css_computed_style_destroy(
								style);
						वापस false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					क्रम (पूर्व = table_row->children;
							पूर्व != शून्य;
							पूर्व = पूर्व->अग्र) {
						अगर (पूर्व->start_column +
							पूर्व->columns ==
								start)
							अवरोध;
						अगर (पूर्व->अग्र == शून्य)
							अवरोध;
					}

					/* Insert it */
					अगर (पूर्व == शून्य) {
						अगर (table_row->children != शून्य)
							table_row->children->
								पूर्व = cell;
						अन्यथा
							table_row->last = cell;

						cell->अग्र =
							table_row->children;
						table_row->children = cell;
					} अन्यथा {
						अगर (पूर्व->अग्र != शून्य)
							पूर्व->अग्र->पूर्व = cell;
						अन्यथा
							table_row->last = cell;

						cell->अग्र = पूर्व->अग्र;
						पूर्व->अग्र = cell;
						cell->पूर्व = पूर्व;
					}
					cell->parent = table_row;
				} अन्यथा {
					spans[col].row_span--;
				}
			}

			निश्चित(rows_बायाँ > 0);

			rows_बायाँ--;
		}

		group_rows_बायाँ--;
	}

	वापस true;
}


अटल bool
box_normalise_table(काष्ठा box *table, स्थिर काष्ठा box *मूल, html_content * c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;
	काष्ठा box *row_group;
	css_computed_style *style;
	काष्ठा columns col_info;
	nscss_select_ctx ctx;

	निश्चित(table != शून्य);
	निश्चित(table->type == BOX_TABLE);

	ctx.मूल_style = मूल->style;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#पूर्ण_अगर

	col_info.अंक_columns = 1;
	col_info.current_column = 0;
	col_info.spans = दो_स्मृति(2 * माप *col_info.spans);
	अगर (col_info.spans == शून्य)
		वापस false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.अंक_rows = 0;

	क्रम (child = table->children; child != शून्य; child = अग्र_child) {
		अग्र_child = child->अग्र;
		चयन (child->type) {
		हाल BOX_TABLE_ROW_GROUP:
			/* ok */
			अगर (box_normalise_table_row_group(child, मूल,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				वापस false;
			}
			अवरोध;
		हाल BOX_FLEX:
		हाल BOX_BLOCK:
		हाल BOX_INLINE_CONTAINER:
		हाल BOX_TABLE:
		हाल BOX_TABLE_ROW:
		हाल BOX_TABLE_CELL:
			/* insert implied table row group */
			निश्चित(table->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			अगर (style == शून्य) {
				मुक्त(col_info.spans);
				वापस false;
			}

			row_group = box_create(शून्य, style, true, table->href,
					table->target, शून्य, शून्य, c->bctx);
			अगर (row_group == शून्य) {
				css_computed_style_destroy(style);
				मुक्त(col_info.spans);
				वापस false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			अगर (child->पूर्व == शून्य)
				table->children = row_group;
			अन्यथा
				child->पूर्व->अग्र = row_group;

			row_group->पूर्व = child->पूर्व;

			जबतक (child != शून्य && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			निश्चित(row_group->last != शून्य);

			row_group->last->अग्र = शून्य;
			row_group->अग्र = अग्र_child = child;
			अगर (row_group->अग्र != शून्य)
				row_group->अग्र->पूर्व = row_group;
			अन्यथा
				table->last = row_group;
			row_group->parent = table;

			अगर (box_normalise_table_row_group(row_group, मूल,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				वापस false;
			}
			अवरोध;
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_INLINE_FLEX:
		हाल BOX_INLINE_BLOCK:
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			निश्चित(0);
			अवरोध;
		शेष:
			ख_लिखो(मानक_त्रुटि, "%i\n", child->type);
			निश्चित(0);
		}
	}

	table->columns = col_info.अंक_columns;
	table->rows = col_info.अंक_rows;

	अगर (table->children == शून्य) {
		काष्ठा box *row;

#अगर_घोषित BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#पूर्ण_अगर

		निश्चित(table->style != शून्य);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		अगर (style == शून्य) {
			मुक्त(col_info.spans);
			वापस false;
		}

		row_group = box_create(शून्य, style, true, table->href,
				table->target, शून्य, शून्य, c->bctx);
		अगर (row_group == शून्य) {
			css_computed_style_destroy(style);
			मुक्त(col_info.spans);
			वापस false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		अगर (style == शून्य) {
			box_free(row_group);
			मुक्त(col_info.spans);
			वापस false;
		}

		row = box_create(शून्य, style, true, row_group->href,
				row_group->target, शून्य, शून्य, c->bctx);
		अगर (row == शून्य) {
			css_computed_style_destroy(style);
			box_free(row_group);
			मुक्त(col_info.spans);
			वापस false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	अगर (box_normalise_table_spans(table, मूल, col_info.spans, c) == false) {
		मुक्त(col_info.spans);
		वापस false;
	}

	मुक्त(col_info.spans);

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#पूर्ण_अगर

	वापस true;
}

अटल bool box_normalise_flex(
		काष्ठा box *flex_container,
		स्थिर काष्ठा box *मूल,
		html_content *c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;
	काष्ठा box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	निश्चित(flex_container != शून्य);
	निश्चित(मूल != शून्य);

	ctx.मूल_style = मूल->style;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#पूर्ण_अगर

	निश्चित(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	क्रम (child = flex_container->children; child != शून्य; child = अग्र_child) {
#अगर_घोषित BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#पूर्ण_अगर

		अग्र_child = child->अग्र;	/* child may be destroyed */

		चयन (child->type) {
		हाल BOX_FLEX:
			/* ok */
			अगर (box_normalise_flex(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_BLOCK:
			/* ok */
			अगर (box_normalise_block(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			निश्चित(flex_container->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			अगर (style == शून्य)
				वापस false;

			implied_flex_item = box_create(शून्य, style, true,
					flex_container->href,
					flex_container->target,
					शून्य, शून्य, c->bctx);
			अगर (implied_flex_item == शून्य) {
				css_computed_style_destroy(style);
				वापस false;
			}
			implied_flex_item->type = BOX_BLOCK;

			अगर (child->पूर्व == शून्य)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->पूर्व->अग्र = implied_flex_item;

			implied_flex_item->पूर्व = child->पूर्व;

			जबतक (child != शून्य &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			implied_flex_item->last->अग्र = शून्य;
			implied_flex_item->अग्र = अग्र_child = child;
			अगर (implied_flex_item->अग्र != शून्य)
				implied_flex_item->अग्र->पूर्व = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			अगर (box_normalise_block(implied_flex_item,
					मूल, c) == false)
				वापस false;
			अवरोध;

		हाल BOX_TABLE:
			अगर (box_normalise_table(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_INLINE_FLEX:
		हाल BOX_INLINE_BLOCK:
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			निश्चित(0);
			अवरोध;
		हाल BOX_TABLE_ROW_GROUP:
		हाल BOX_TABLE_ROW:
		हाल BOX_TABLE_CELL:
			/* insert implied table */
			निश्चित(flex_container->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			अगर (style == शून्य)
				वापस false;

			implied_flex_item = box_create(शून्य, style, true,
					flex_container->href,
					flex_container->target,
					शून्य, शून्य, c->bctx);
			अगर (implied_flex_item == शून्य) {
				css_computed_style_destroy(style);
				वापस false;
			}
			implied_flex_item->type = BOX_TABLE;

			अगर (child->पूर्व == शून्य)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->पूर्व->अग्र = implied_flex_item;

			implied_flex_item->पूर्व = child->पूर्व;

			जबतक (child != शून्य && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			implied_flex_item->last->अग्र = शून्य;
			implied_flex_item->अग्र = अग्र_child = child;
			अगर (implied_flex_item->अग्र != शून्य)
				implied_flex_item->अग्र->पूर्व = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			अगर (box_normalise_table(implied_flex_item,
					मूल, c) == false)
				वापस false;
			अवरोध;
		शेष:
			निश्चित(0);
		}
	}

	वापस true;
}

अटल bool
box_normalise_inline_container(काष्ठा box *cont,
			       स्थिर काष्ठा box *मूल,
			       html_content * c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;

	निश्चित(cont != शून्य);
	निश्चित(cont->type == BOX_INLINE_CONTAINER);

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#पूर्ण_अगर

	क्रम (child = cont->children; child != शून्य; child = अग्र_child) {
		अग्र_child = child->अग्र;
		चयन (child->type) {
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* ok */
			अवरोध;
		हाल BOX_INLINE_BLOCK:
			/* ok */
			अगर (box_normalise_block(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE_FLEX:
			/* ok */
			अगर (box_normalise_flex(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
			/* ok */
			निश्चित(child->children != शून्य);

			चयन (child->children->type) {
			हाल BOX_BLOCK:
				अगर (box_normalise_block(child->children, मूल,
						c) == false)
					वापस false;
				अवरोध;
			हाल BOX_TABLE:
				अगर (box_normalise_table(child->children, मूल,
						c) == false)
					वापस false;
				अवरोध;
			हाल BOX_FLEX:
				अगर (box_normalise_flex(child->children, मूल,
						c) == false)
					वापस false;
				अवरोध;
			शेष:
				निश्चित(0);
			}

			अगर (child->children == शून्य) {
				/* the child has destroyed itself: remove float */
				अगर (child->पूर्व == शून्य)
					child->parent->children = child->अग्र;
				अन्यथा
					child->पूर्व->अग्र = child->अग्र;
				अगर (child->अग्र != शून्य)
					child->अग्र->पूर्व = child->पूर्व;
				अन्यथा
					child->parent->last = child->पूर्व;

				box_free(child);
			}
			अवरोध;
		हाल BOX_FLEX:
		हाल BOX_BLOCK:
		हाल BOX_INLINE_CONTAINER:
		हाल BOX_TABLE:
		हाल BOX_TABLE_ROW_GROUP:
		हाल BOX_TABLE_ROW:
		हाल BOX_TABLE_CELL:
		शेष:
			निश्चित(0);
		}
	}

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#पूर्ण_अगर

	वापस true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(काष्ठा box *block, स्थिर काष्ठा box *मूल, html_content *c)
{
	काष्ठा box *child;
	काष्ठा box *अग्र_child;
	काष्ठा box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	निश्चित(block != शून्य);
	निश्चित(मूल != शून्य);

	ctx.मूल_style = मूल->style;

#अगर_घोषित BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#पूर्ण_अगर

	निश्चित(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	क्रम (child = block->children; child != शून्य; child = अग्र_child) {
#अगर_घोषित BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#पूर्ण_अगर

		अग्र_child = child->अग्र;	/* child may be destroyed */

		चयन (child->type) {
		हाल BOX_FLEX:
			/* ok */
			अगर (box_normalise_flex(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_BLOCK:
			/* ok */
			अगर (box_normalise_block(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE_CONTAINER:
			अगर (box_normalise_inline_container(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_TABLE:
			अगर (box_normalise_table(child, मूल, c) == false)
				वापस false;
			अवरोध;
		हाल BOX_INLINE:
		हाल BOX_INLINE_END:
		हाल BOX_INLINE_FLEX:
		हाल BOX_INLINE_BLOCK:
		हाल BOX_FLOAT_बायाँ:
		हाल BOX_FLOAT_दायाँ:
		हाल BOX_BR:
		हाल BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			निश्चित(0);
			अवरोध;
		हाल BOX_TABLE_ROW_GROUP:
		हाल BOX_TABLE_ROW:
		हाल BOX_TABLE_CELL:
			/* insert implied table */
			निश्चित(block->style != शून्य);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			अगर (style == शून्य)
				वापस false;

			table = box_create(शून्य, style, true, block->href,
					block->target, शून्य, शून्य, c->bctx);
			अगर (table == शून्य) {
				css_computed_style_destroy(style);
				वापस false;
			}
			table->type = BOX_TABLE;

			अगर (child->पूर्व == शून्य)
				block->children = table;
			अन्यथा
				child->पूर्व->अग्र = table;

			table->पूर्व = child->पूर्व;

			जबतक (child != शून्य && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				अग्र_child = child->अग्र;
				child->अग्र = शून्य;
				child = अग्र_child;
			}

			table->last->अग्र = शून्य;
			table->अग्र = अग्र_child = child;
			अगर (table->अग्र != शून्य)
				table->अग्र->पूर्व = table;
			अन्यथा
				block->last = table;
			table->parent = block;

			अगर (box_normalise_table(table, मूल, c) == false)
				वापस false;
			अवरोध;
		शेष:
			निश्चित(0);
		}
	}

	वापस true;
}
