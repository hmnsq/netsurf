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
#अपरिभाषय BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
संरचना विस्तृतिः_info {
	/** Number of rows this cell spans */
	अचिह्नित पूर्णाङ्क row_विस्तृतिः;
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
	अचिह्नित पूर्णाङ्क current_column;
	/** Number of columns in main part of table 1..max columns */
	अचिह्नित पूर्णाङ्क अङ्कः_columns;
	/** Information about columns in main table, array [0, num_columns) */
	संरचना विस्तृतिः_info *spans;
	/** Number of rows in table */
	अचिह्नित पूर्णाङ्क अङ्कः_पङ्क्तिकाः;
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
calculate_सारणी_row(संरचना columns *col_info,
		    अचिह्नित पूर्णाङ्क col_विस्तृतिः,
		    अचिह्नित पूर्णाङ्क row_विस्तृतिः,
		    अचिह्नित पूर्णाङ्क *start_column,
		    संरचना box *cell)
{
	अचिह्नित पूर्णाङ्क cell_start_col = col_info->current_column;
	अचिह्नित पूर्णाङ्क cell_अन्तः_col;
	अचिह्नित पूर्णाङ्क i;
	संरचना विस्तृतिः_info *spans;
	संरचना box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	यावत् (col_info->spans[cell_start_col].row_विस्तृतिः != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	यदि (col_विस्तृतिः == 0)
		col_विस्तृतिः = 1;

	cell_अन्तः_col = cell_start_col + col_विस्तृतिः;

	यदि (col_info->अङ्कः_columns < cell_अन्तः_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				परिमाण *spans * (cell_अन्तः_col + 1));
		यदि (spans == रिक्त)
			प्रत्यागम false;

		col_info->spans = spans;
		col_info->अङ्कः_columns = cell_अन्तः_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_अन्तः_col].row_विस्तृतिः = 0;
		col_info->spans[cell_अन्तः_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	प्रति (i = cell_start_col; i < cell_अन्तः_col; i++) {
		col_info->spans[i].row_विस्तृतिः = (row_विस्तृतिः == 0) ? 1 : row_विस्तृतिः;
		col_info->spans[i].auto_row = (row_विस्तृतिः == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_अन्तः_col;

	*start_column = cell_start_col;

	प्रत्यागम true;
}


स्थिर bool
box_normalise_सारणी_row(संरचना box *row,
			ध्रुव संरचना box *मूलम्,
			संरचना columns *col_info,
			html_content * c)
{
	संरचना box *child;
	संरचना box *अग्र_child;
	संरचना box *cell = रिक्त;
	css_computed_style *style;
	अचिह्नित पूर्णाङ्क i;
	nscss_select_ctx ctx;

	प्रतिज्ञा(row != रिक्त);
	प्रतिज्ञा(row->type == BOX_सारणी_ROW);

	ctx.मूलम्_style = मूलम्->style;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#यदि_अन्तः

	प्रति (child = row->children; child != रिक्त; child = अग्र_child) {
		अग्र_child = child->अग्र;

		विकल्प (child->type) {
		पक्ष BOX_सारणी_CELL:
			/* ok */
			यदि (box_normalise_खण्डम्(child, मूलम्, c) == false)
				प्रत्यागम false;
			cell = child;
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_खण्डम्:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_सारणी:
		पक्ष BOX_सारणी_ROW_गणः:
		पक्ष BOX_सारणी_ROW:
			/* insert implied table cell */
			प्रतिज्ञा(row->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
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
			cell->type = BOX_सारणी_CELL;

			यदि (child->पूर्व == रिक्त)
				row->children = cell;
			अन्यथा
				child->पूर्व->अग्र = cell;

			cell->पूर्व = child->पूर्व;

			यावत् (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_खण्डम् ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_सारणी ||
					child->type == BOX_सारणी_ROW_गणः ||
					child->type == BOX_सारणी_ROW)) {
				box_सङ्कलय_child(cell, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			प्रतिज्ञा(cell->चरमः != रिक्त);

			cell->चरमः->अग्र = रिक्त;
			cell->अग्र = अग्र_child = child;
			यदि (cell->अग्र != रिक्त)
				cell->अग्र->पूर्व = cell;
			अन्यथा
				row->चरमः = cell;
			cell->parent = row;

			यदि (box_normalise_खण्डम्(cell, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_खण्डम्:
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}

		यदि (calculate_सारणी_row(col_info, cell->columns, cell->पङ्क्तिकाः,
				&cell->start_column, cell) == false)
			प्रत्यागम false;
	}


	/* Update row spanning details for all columns */
	प्रति (i = 0; i < col_info->अङ्कः_columns; i++) {
		यदि (col_info->spans[i].row_विस्तृतिः != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_विस्तृतिः--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->अङ्कः_पङ्क्तिकाः++;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#यदि_अन्तः

	प्रत्यागम true;
}


स्थिर bool
box_normalise_सारणी_row_गणः(संरचना box *row_गणः,
			      ध्रुव संरचना box *मूलम्,
			      संरचना columns *col_info,
			      html_content * c)
{
	संरचना box *child;
	संरचना box *अग्र_child;
	संरचना box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	अचिह्नित पूर्णाङ्क गणः_row_गणनम् = 0;

	प्रतिज्ञा(row_गणः != 0);
	प्रतिज्ञा(row_गणः->type == BOX_सारणी_ROW_गणः);

	ctx.मूलम्_style = मूलम्->style;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_गणः);
#यदि_अन्तः

	प्रति (child = row_गणः->children; child != रिक्त; child = अग्र_child) {
		अग्र_child = child->अग्र;

		विकल्प (child->type) {
		पक्ष BOX_सारणी_ROW:
			/* ok */
			गणः_row_गणनम्++;
			यदि (box_normalise_सारणी_row(child, मूलम्, col_info,
					c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_खण्डम्:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_सारणी:
		पक्ष BOX_सारणी_ROW_गणः:
		पक्ष BOX_सारणी_CELL:
			/* insert implied table row */
			प्रतिज्ञा(row_गणः->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_गणः->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			row = box_create(रिक्त, style, true, row_गणः->href,
					row_गणः->target, रिक्त, रिक्त, c->bctx);
			यदि (row == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			row->type = BOX_सारणी_ROW;

			यदि (child->पूर्व == रिक्त)
				row_गणः->children = row;
			अन्यथा
				child->पूर्व->अग्र = row;

			row->पूर्व = child->पूर्व;

			यावत् (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_खण्डम् ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_सारणी ||
					child->type == BOX_सारणी_ROW_गणः ||
					child->type == BOX_सारणी_CELL)) {
				box_सङ्कलय_child(row, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			प्रतिज्ञा(row->चरमः != रिक्त);

			row->चरमः->अग्र = रिक्त;
			row->अग्र = अग्र_child = child;
			यदि (row->अग्र != रिक्त)
				row->अग्र->पूर्व = row;
			अन्यथा
				row_गणः->चरमः = row;
			row->parent = row_गणः;

			गणः_row_गणनम्++;
			यदि (box_normalise_सारणी_row(row, मूलम्, col_info,
					c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_खण्डम्:
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
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

	यदि (row_गणः->children == रिक्त) {
#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#यदि_अन्तः

		प्रतिज्ञा(row_गणः->style != रिक्त);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
		ctx.आधारः_url = c->आधारः_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_गणः->style);
		यदि (style == रिक्त) {
			प्रत्यागम false;
		}

		row = box_create(रिक्त, style, true, row_गणः->href,
				row_गणः->target, रिक्त, रिक्त, c->bctx);
		यदि (row == रिक्त) {
			css_computed_style_destroy(style);
			प्रत्यागम false;
		}
		row->type = BOX_सारणी_ROW;

		row->parent = row_गणः;
		row_गणः->children = row_गणः->चरमः = row;

		गणः_row_गणनम् = 1;

		/* Keep table's row count in sync */
		col_info->अङ्कः_पङ्क्तिकाः++;
	}

	row_गणः->पङ्क्तिकाः = गणः_row_गणनम्;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_गणः);
#यदि_अन्तः

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
box_normalise_सारणी_spans(संरचना box *सारणी,
			  ध्रुव संरचना box *मूलम्,
			  संरचना विस्तृतिः_info *spans,
			  html_content *c)
{
	संरचना box *सारणी_row_गणः;
	संरचना box *सारणी_row;
	संरचना box *सारणी_cell;
	अचिह्नित पूर्णाङ्क पङ्क्तिकाः_वामः = सारणी->पङ्क्तिकाः;
	अचिह्नित पूर्णाङ्क गणः_पङ्क्तिकाः_वामः;
	अचिह्नित पूर्णाङ्क col;
	nscss_select_ctx ctx;

	ctx.मूलम्_style = मूलम्->style;

	/* Clear span data */
	स्मृतिं_पूरय(spans, 0, सारणी->columns * परिमाण(संरचना विस्तृतिः_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	प्रति (सारणी_row_गणः = सारणी->children;
	     सारणी_row_गणः != रिक्त;
	     सारणी_row_गणः = सारणी_row_गणः->अग्र) {

		गणः_पङ्क्तिकाः_वामः = सारणी_row_गणः->पङ्क्तिकाः;

		प्रति (सारणी_row = सारणी_row_गणः->children;
		     सारणी_row != रिक्त;
		     सारणी_row = सारणी_row->अग्र) {

			प्रति (सारणी_cell = सारणी_row->children;
			     सारणी_cell != रिक्त;
			     सारणी_cell = सारणी_cell->अग्र) {

				/* colspan = 0 -> colspan = 1 */
				यदि (सारणी_cell->columns == 0) {
					सारणी_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				यदि (सारणी_cell->पङ्क्तिकाः == 0) {
					सारणी_cell->पङ्क्तिकाः = गणः_पङ्क्तिकाः_वामः;
				}

				/* limit rowspans within group */
				यदि (सारणी_cell->पङ्क्तिकाः > गणः_पङ्क्तिकाः_वामः) {
					सारणी_cell->पङ्क्तिकाः = गणः_पङ्क्तिकाः_वामः;
				}

				/* Record span information */
				प्रति (col = सारणी_cell->start_column;
						col < सारणी_cell->start_column +
						सारणी_cell->columns; col++) {
					spans[col].row_विस्तृतिः = सारणी_cell->पङ्क्तिकाः;
				}
			}

			/* Reduce span count of each column */
			प्रति (col = 0; col < सारणी->columns; col++) {
				यदि (spans[col].row_विस्तृतिः == 0) {
					अचिह्नित पूर्णाङ्क start = col;
					css_computed_style *style;
					संरचना box *cell, *पूर्व;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					प्रतिज्ञा(सारणी_row->style != रिक्त);

					/* Find width of gap */
					यावत् (col < सारणी->columns &&
							spans[col].row_विस्तृतिः ==
							0) {
						col++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
					ctx.आधारः_url = c->आधारः_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							सारणी_row->style);
					यदि (style == रिक्त)
						प्रत्यागम false;

					cell = box_create(रिक्त, style, true,
							सारणी_row->href,
							सारणी_row->target,
							रिक्त, रिक्त, c->bctx);
					यदि (cell == रिक्त) {
						css_computed_style_destroy(
								style);
						प्रत्यागम false;
					}
					cell->type = BOX_सारणी_CELL;

					cell->पङ्क्तिकाः = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					प्रति (पूर्व = सारणी_row->children;
							पूर्व != रिक्त;
							पूर्व = पूर्व->अग्र) {
						यदि (पूर्व->start_column +
							पूर्व->columns ==
								start)
							भङ्ग;
						यदि (पूर्व->अग्र == रिक्त)
							भङ्ग;
					}

					/* Insert it */
					यदि (पूर्व == रिक्त) {
						यदि (सारणी_row->children != रिक्त)
							सारणी_row->children->
								पूर्व = cell;
						अन्यथा
							सारणी_row->चरमः = cell;

						cell->अग्र =
							सारणी_row->children;
						सारणी_row->children = cell;
					} अन्यथा {
						यदि (पूर्व->अग्र != रिक्त)
							पूर्व->अग्र->पूर्व = cell;
						अन्यथा
							सारणी_row->चरमः = cell;

						cell->अग्र = पूर्व->अग्र;
						पूर्व->अग्र = cell;
						cell->पूर्व = पूर्व;
					}
					cell->parent = सारणी_row;
				} अन्यथा {
					spans[col].row_विस्तृतिः--;
				}
			}

			प्रतिज्ञा(पङ्क्तिकाः_वामः > 0);

			पङ्क्तिकाः_वामः--;
		}

		गणः_पङ्क्तिकाः_वामः--;
	}

	प्रत्यागम true;
}


स्थिर bool
box_normalise_सारणी(संरचना box *सारणी, ध्रुव संरचना box *मूलम्, html_content * c)
{
	संरचना box *child;
	संरचना box *अग्र_child;
	संरचना box *row_गणः;
	css_computed_style *style;
	संरचना columns col_info;
	nscss_select_ctx ctx;

	प्रतिज्ञा(सारणी != रिक्त);
	प्रतिज्ञा(सारणी->type == BOX_सारणी);

	ctx.मूलम्_style = मूलम्->style;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", सारणी);
#यदि_अन्तः

	col_info.अङ्कः_columns = 1;
	col_info.current_column = 0;
	col_info.spans = स्मृतिं_देहि(2 * परिमाण *col_info.spans);
	यदि (col_info.spans == रिक्त)
		प्रत्यागम false;

	col_info.spans[0].row_विस्तृतिः = col_info.spans[1].row_विस्तृतिः = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.अङ्कः_पङ्क्तिकाः = 0;

	प्रति (child = सारणी->children; child != रिक्त; child = अग्र_child) {
		अग्र_child = child->अग्र;
		विकल्प (child->type) {
		पक्ष BOX_सारणी_ROW_गणः:
			/* ok */
			यदि (box_normalise_सारणी_row_गणः(child, मूलम्,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_खण्डम्:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_सारणी:
		पक्ष BOX_सारणी_ROW:
		पक्ष BOX_सारणी_CELL:
			/* insert implied table row group */
			प्रतिज्ञा(सारणी->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					सारणी->style);
			यदि (style == रिक्त) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}

			row_गणः = box_create(रिक्त, style, true, सारणी->href,
					सारणी->target, रिक्त, रिक्त, c->bctx);
			यदि (row_गणः == रिक्त) {
				css_computed_style_destroy(style);
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}

			row_गणः->type = BOX_सारणी_ROW_गणः;

			यदि (child->पूर्व == रिक्त)
				सारणी->children = row_गणः;
			अन्यथा
				child->पूर्व->अग्र = row_गणः;

			row_गणः->पूर्व = child->पूर्व;

			यावत् (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_खण्डम् ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_सारणी ||
					child->type == BOX_सारणी_ROW ||
					child->type == BOX_सारणी_CELL)) {
				box_सङ्कलय_child(row_गणः, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			प्रतिज्ञा(row_गणः->चरमः != रिक्त);

			row_गणः->चरमः->अग्र = रिक्त;
			row_गणः->अग्र = अग्र_child = child;
			यदि (row_गणः->अग्र != रिक्त)
				row_गणः->अग्र->पूर्व = row_गणः;
			अन्यथा
				सारणी->चरमः = row_गणः;
			row_गणः->parent = सारणी;

			यदि (box_normalise_सारणी_row_गणः(row_गणः, मूलम्,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				प्रत्यागम false;
			}
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_खण्डम्:
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
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

	सारणी->columns = col_info.अङ्कः_columns;
	सारणी->पङ्क्तिकाः = col_info.अङ्कः_पङ्क्तिकाः;

	यदि (सारणी->children == रिक्त) {
		संरचना box *row;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#यदि_अन्तः

		प्रतिज्ञा(सारणी->style != रिक्त);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
		ctx.आधारः_url = c->आधारः_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				सारणी->style);
		यदि (style == रिक्त) {
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}

		row_गणः = box_create(रिक्त, style, true, सारणी->href,
				सारणी->target, रिक्त, रिक्त, c->bctx);
		यदि (row_गणः == रिक्त) {
			css_computed_style_destroy(style);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}
		row_गणः->type = BOX_सारणी_ROW_गणः;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_गणः->style);
		यदि (style == रिक्त) {
			box_free(row_गणः);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}

		row = box_create(रिक्त, style, true, row_गणः->href,
				row_गणः->target, रिक्त, रिक्त, c->bctx);
		यदि (row == रिक्त) {
			css_computed_style_destroy(style);
			box_free(row_गणः);
			मुक्त(col_info.spans);
			प्रत्यागम false;
		}
		row->type = BOX_सारणी_ROW;

		row->parent = row_गणः;
		row_गणः->children = row_गणः->चरमः = row;

		row_गणः->parent = सारणी;
		सारणी->children = सारणी->चरमः = row_गणः;

		सारणी->पङ्क्तिकाः = 1;
	}

	यदि (box_normalise_सारणी_spans(सारणी, मूलम्, col_info.spans, c) == false) {
		मुक्त(col_info.spans);
		प्रत्यागम false;
	}

	मुक्त(col_info.spans);

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", सारणी);
#यदि_अन्तः

	प्रत्यागम true;
}

स्थिर bool box_normalise_flex(
		संरचना box *flex_container,
		ध्रुव संरचना box *मूलम्,
		html_content *c)
{
	संरचना box *child;
	संरचना box *अग्र_child;
	संरचना box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(flex_container != रिक्त);
	प्रतिज्ञा(मूलम् != रिक्त);

	ctx.मूलम्_style = मूलम्->style;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#यदि_अन्तः

	प्रतिज्ञा(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	प्रति (child = flex_container->children; child != रिक्त; child = अग्र_child) {
#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#यदि_अन्तः

		अग्र_child = child->अग्र;	/* child may be destroyed */

		विकल्प (child->type) {
		पक्ष BOX_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_खण्डम्:
			/* ok */
			यदि (box_normalise_खण्डम्(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
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
			implied_flex_item->type = BOX_खण्डम्;

			यदि (child->पूर्व == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->पूर्व->अग्र = implied_flex_item;

			implied_flex_item->पूर्व = child->पूर्व;

			यावत् (child != रिक्त &&
					child->type == BOX_INLINE_CONTAINER) {
				box_सङ्कलय_child(implied_flex_item, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			implied_flex_item->चरमः->अग्र = रिक्त;
			implied_flex_item->अग्र = अग्र_child = child;
			यदि (implied_flex_item->अग्र != रिक्त)
				implied_flex_item->अग्र->पूर्व = implied_flex_item;
			अन्यथा
				flex_container->चरमः = implied_flex_item;
			implied_flex_item->parent = flex_container;

			यदि (box_normalise_खण्डम्(implied_flex_item,
					मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;

		पक्ष BOX_सारणी:
			यदि (box_normalise_सारणी(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_खण्डम्:
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		पक्ष BOX_सारणी_ROW_गणः:
		पक्ष BOX_सारणी_ROW:
		पक्ष BOX_सारणी_CELL:
			/* insert implied table */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
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
			implied_flex_item->type = BOX_सारणी;

			यदि (child->पूर्व == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->पूर्व->अग्र = implied_flex_item;

			implied_flex_item->पूर्व = child->पूर्व;

			यावत् (child != रिक्त && (
					child->type == BOX_सारणी_ROW_गणः ||
					child->type == BOX_सारणी_ROW ||
					child->type == BOX_सारणी_CELL)) {
				box_सङ्कलय_child(implied_flex_item, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			implied_flex_item->चरमः->अग्र = रिक्त;
			implied_flex_item->अग्र = अग्र_child = child;
			यदि (implied_flex_item->अग्र != रिक्त)
				implied_flex_item->अग्र->पूर्व = implied_flex_item;
			अन्यथा
				flex_container->चरमः = implied_flex_item;
			implied_flex_item->parent = flex_container;

			यदि (box_normalise_सारणी(implied_flex_item,
					मूलम्, c) == false)
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
			       ध्रुव संरचना box *मूलम्,
			       html_content * c)
{
	संरचना box *child;
	संरचना box *अग्र_child;

	प्रतिज्ञा(cont != रिक्त);
	प्रतिज्ञा(cont->type == BOX_INLINE_CONTAINER);

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#यदि_अन्तः

	प्रति (child = cont->children; child != रिक्त; child = अग्र_child) {
		अग्र_child = child->अग्र;
		विकल्प (child->type) {
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* ok */
			भङ्ग;
		पक्ष BOX_INLINE_खण्डम्:
			/* ok */
			यदि (box_normalise_खण्डम्(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
			/* ok */
			प्रतिज्ञा(child->children != रिक्त);

			विकल्प (child->children->type) {
			पक्ष BOX_खण्डम्:
				यदि (box_normalise_खण्डम्(child->children, मूलम्,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			पक्ष BOX_सारणी:
				यदि (box_normalise_सारणी(child->children, मूलम्,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			पक्ष BOX_FLEX:
				यदि (box_normalise_flex(child->children, मूलम्,
						c) == false)
					प्रत्यागम false;
				भङ्ग;
			स्वभाव:
				प्रतिज्ञा(0);
			}

			यदि (child->children == रिक्त) {
				/* the child has destroyed itself: remove float */
				यदि (child->पूर्व == रिक्त)
					child->parent->children = child->अग्र;
				अन्यथा
					child->पूर्व->अग्र = child->अग्र;
				यदि (child->अग्र != रिक्त)
					child->अग्र->पूर्व = child->पूर्व;
				अन्यथा
					child->parent->चरमः = child->पूर्व;

				box_free(child);
			}
			भङ्ग;
		पक्ष BOX_FLEX:
		पक्ष BOX_खण्डम्:
		पक्ष BOX_INLINE_CONTAINER:
		पक्ष BOX_सारणी:
		पक्ष BOX_सारणी_ROW_गणः:
		पक्ष BOX_सारणी_ROW:
		पक्ष BOX_सारणी_CELL:
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#यदि_अन्तः

	प्रत्यागम true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_खण्डम्(संरचना box *खण्डम्, ध्रुव संरचना box *मूलम्, html_content *c)
{
	संरचना box *child;
	संरचना box *अग्र_child;
	संरचना box *सारणी;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(खण्डम् != रिक्त);
	प्रतिज्ञा(मूलम् != रिक्त);

	ctx.मूलम्_style = मूलम्->style;

#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", खण्डम्, खण्डम्->type);
#यदि_अन्तः

	प्रतिज्ञा(खण्डम्->type == BOX_खण्डम् || खण्डम्->type == BOX_INLINE_खण्डम् ||
			खण्डम्->type == BOX_सारणी_CELL);

	प्रति (child = खण्डम्->children; child != रिक्त; child = अग्र_child) {
#यदि_परिभाषितम् BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#यदि_अन्तः

		अग्र_child = child->अग्र;	/* child may be destroyed */

		विकल्प (child->type) {
		पक्ष BOX_FLEX:
			/* ok */
			यदि (box_normalise_flex(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_खण्डम्:
			/* ok */
			यदि (box_normalise_खण्डम्(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE_CONTAINER:
			यदि (box_normalise_inline_container(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_सारणी:
			यदि (box_normalise_सारणी(child, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		पक्ष BOX_INLINE:
		पक्ष BOX_INLINE_अन्तः:
		पक्ष BOX_INLINE_FLEX:
		पक्ष BOX_INLINE_खण्डम्:
		पक्ष BOX_FLOAT_वामः:
		पक्ष BOX_FLOAT_दक्षिणः:
		पक्ष BOX_BR:
		पक्ष BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			भङ्ग;
		पक्ष BOX_सारणी_ROW_गणः:
		पक्ष BOX_सारणी_ROW:
		पक्ष BOX_सारणी_CELL:
			/* insert implied table */
			प्रतिज्ञा(खण्डम्->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्);
			ctx.आधारः_url = c->आधारः_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					खण्डम्->style);
			यदि (style == रिक्त)
				प्रत्यागम false;

			सारणी = box_create(रिक्त, style, true, खण्डम्->href,
					खण्डम्->target, रिक्त, रिक्त, c->bctx);
			यदि (सारणी == रिक्त) {
				css_computed_style_destroy(style);
				प्रत्यागम false;
			}
			सारणी->type = BOX_सारणी;

			यदि (child->पूर्व == रिक्त)
				खण्डम्->children = सारणी;
			अन्यथा
				child->पूर्व->अग्र = सारणी;

			सारणी->पूर्व = child->पूर्व;

			यावत् (child != रिक्त && (
					child->type == BOX_सारणी_ROW_गणः ||
					child->type == BOX_सारणी_ROW ||
					child->type == BOX_सारणी_CELL)) {
				box_सङ्कलय_child(सारणी, child);

				अग्र_child = child->अग्र;
				child->अग्र = रिक्त;
				child = अग्र_child;
			}

			सारणी->चरमः->अग्र = रिक्त;
			सारणी->अग्र = अग्र_child = child;
			यदि (सारणी->अग्र != रिक्त)
				सारणी->अग्र->पूर्व = सारणी;
			अन्यथा
				खण्डम्->चरमः = सारणी;
			सारणी->parent = खण्डम्;

			यदि (box_normalise_सारणी(सारणी, मूलम्, c) == false)
				प्रत्यागम false;
			भङ्ग;
		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

	प्रत्यागम true;
}
