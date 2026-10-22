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
#تعریف_منسوخ BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ساخت وسعت_info {
	/** Number of rows this cell spans */
	غیرعلامتی صحیح صف_وسعت;
	/** Row group of cell */
	ساخت box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_صف;
};

/**
 * Column record for a table
 */
ساخت columns {
	/** Current column index */
	غیرعلامتی صحیح current_عمود;
	/** Number of columns in main part of table 1..max columns */
	غیرعلامتی صحیح عدد_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ساخت وسعت_info *spans;
	/** Number of rows in table */
	غیرعلامتی صحیح عدد_صفیں;
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
ساکن bool
calculate_جدول_صف(ساخت columns *ستون_info,
		    غیرعلامتی صحیح ستون_وسعت,
		    غیرعلامتی صحیح صف_وسعت,
		    غیرعلامتی صحیح *آغاز_عمود,
		    ساخت box *cell)
{
	غیرعلامتی صحیح cell_آغاز_ستون = ستون_info->current_عمود;
	غیرعلامتی صحیح cell_انتہا_ستون;
	غیرعلامتی صحیح i;
	ساخت وسعت_info *spans;
	ساخت box *rg = cell->والد->والد; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	جب (ستون_info->spans[cell_آغاز_ستون].صف_وسعت != 0 &&
			ستون_info->spans[cell_آغاز_ستون].rg == rg) {
		cell_آغاز_ستون++;
	}

	/* Update current column with calculated start */
	ستون_info->current_عمود = cell_آغاز_ستون;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	اگر (ستون_وسعت == 0)
		ستون_وسعت = 1;

	cell_انتہا_ستون = cell_آغاز_ستون + ستون_وسعت;

	اگر (ستون_info->عدد_columns < cell_انتہا_ستون) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(ستون_info->spans,
				حجم *spans * (cell_انتہا_ستون + 1));
		اگر (spans == تہی)
			واپس false;

		ستون_info->spans = spans;
		ستون_info->عدد_columns = cell_انتہا_ستون;

		/* Mark new final column as sentinel */
		ستون_info->spans[cell_انتہا_ستون].صف_وسعت = 0;
		ستون_info->spans[cell_انتہا_ستون].auto_صف = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	برائے (i = cell_آغاز_ستون; i < cell_انتہا_ستون; i++) {
		ستون_info->spans[i].صف_وسعت = (صف_وسعت == 0) ? 1 : صف_وسعت;
		ستون_info->spans[i].auto_صف = (صف_وسعت == 0);
		ستون_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	ستون_info->current_عمود = cell_انتہا_ستون;

	*آغاز_عمود = cell_آغاز_ستون;

	واپس true;
}


ساکن bool
box_normalise_جدول_صف(ساخت box *صف,
			ثابت ساخت box *جڑ,
			ساخت columns *ستون_info,
			html_content * c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;
	ساخت box *cell = تہی;
	css_computed_style *style;
	غیرعلامتی صحیح i;
	nscss_select_ctx ctx;

	أكد(صف != تہی);
	أكد(صف->type == BOX_جدول_صف);

	ctx.جڑ_style = جڑ->style;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", صف);
#اگر_ختم

	برائے (بچہ = صف->children; بچہ != تہی; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;

		انتخاب (بچہ->type) {
		حالت BOX_جدول_CELL:
			/* ok */
			اگر (box_normalise_بلاک(بچہ, جڑ, c) == false)
				واپس false;
			cell = بچہ;
			توڑو;
		حالت BOX_FLEX:
		حالت BOX_بلاک:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_جدول:
		حالت BOX_جدول_صف_گروہ:
		حالت BOX_جدول_صف:
			/* insert implied table cell */
			أكد(صف->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف->style);
			اگر (style == تہی)
				واپس false;

			cell = box_create(تہی, style, true, صف->href,
					صف->ہدف, تہی, تہی, c->bctx);
			اگر (cell == تہی) {
				css_computed_style_destroy(style);
				واپس false;
			}
			cell->type = BOX_جدول_CELL;

			اگر (بچہ->پچھلا == تہی)
				صف->children = cell;
			ورنہ
				بچہ->پچھلا->اگلا = cell;

			cell->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف)) {
				box_جوڑو_بچہ(cell, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			أكد(cell->آخری != تہی);

			cell->آخری->اگلا = تہی;
			cell->اگلا = اگلا_بچہ = بچہ;
			اگر (cell->اگلا != تہی)
				cell->اگلا->پچھلا = cell;
			ورنہ
				صف->آخری = cell;
			cell->والد = صف;

			اگر (box_normalise_بلاک(cell, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_بلاک:
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
		حالت BOX_BR:
		حالت BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			توڑو;
		طےشدہ:
			أكد(0);
		}

		اگر (calculate_جدول_صف(ستون_info, cell->columns, cell->صفیں,
				&cell->آغاز_عمود, cell) == false)
			واپس false;
	}


	/* Update row spanning details for all columns */
	برائے (i = 0; i < ستون_info->عدد_columns; i++) {
		اگر (ستون_info->spans[i].صف_وسعت != 0 &&
				ستون_info->spans[i].auto_صف == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			ستون_info->spans[i].صف_وسعت--;
		}
	}

	/* Reset current column for next row */
	ستون_info->current_عمود = 0;

	/* Increment row counter */
	ستون_info->عدد_صفیں++;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", صف);
#اگر_ختم

	واپس true;
}


ساکن bool
box_normalise_جدول_صف_گروہ(ساخت box *صف_گروہ,
			      ثابت ساخت box *جڑ,
			      ساخت columns *ستون_info,
			      html_content * c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;
	ساخت box *صف;
	css_computed_style *style;
	nscss_select_ctx ctx;
	غیرعلامتی صحیح گروہ_صف_گنتی = 0;

	أكد(صف_گروہ != 0);
	أكد(صف_گروہ->type == BOX_جدول_صف_گروہ);

	ctx.جڑ_style = جڑ->style;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", صف_گروہ);
#اگر_ختم

	برائے (بچہ = صف_گروہ->children; بچہ != تہی; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;

		انتخاب (بچہ->type) {
		حالت BOX_جدول_صف:
			/* ok */
			گروہ_صف_گنتی++;
			اگر (box_normalise_جدول_صف(بچہ, جڑ, ستون_info,
					c) == false)
				واپس false;
			توڑو;
		حالت BOX_FLEX:
		حالت BOX_بلاک:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_جدول:
		حالت BOX_جدول_صف_گروہ:
		حالت BOX_جدول_CELL:
			/* insert implied table row */
			أكد(صف_گروہ->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف_گروہ->style);
			اگر (style == تہی)
				واپس false;

			صف = box_create(تہی, style, true, صف_گروہ->href,
					صف_گروہ->ہدف, تہی, تہی, c->bctx);
			اگر (صف == تہی) {
				css_computed_style_destroy(style);
				واپس false;
			}
			صف->type = BOX_جدول_صف;

			اگر (بچہ->پچھلا == تہی)
				صف_گروہ->children = صف;
			ورنہ
				بچہ->پچھلا->اگلا = صف;

			صف->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(صف, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			أكد(صف->آخری != تہی);

			صف->آخری->اگلا = تہی;
			صف->اگلا = اگلا_بچہ = بچہ;
			اگر (صف->اگلا != تہی)
				صف->اگلا->پچھلا = صف;
			ورنہ
				صف_گروہ->آخری = صف;
			صف->والد = صف_گروہ;

			گروہ_صف_گنتی++;
			اگر (box_normalise_جدول_صف(صف, جڑ, ستون_info,
					c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_بلاک:
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
		حالت BOX_BR:
		حالت BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			توڑو;
		طےشدہ:
			أكد(0);
		}
	}

	اگر (صف_گروہ->children == تہی) {
#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#اگر_ختم

		أكد(صف_گروہ->style != تہی);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
		ctx.بنیاد_url = c->بنیاد_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروہ->style);
		اگر (style == تہی) {
			واپس false;
		}

		صف = box_create(تہی, style, true, صف_گروہ->href,
				صف_گروہ->ہدف, تہی, تہی, c->bctx);
		اگر (صف == تہی) {
			css_computed_style_destroy(style);
			واپس false;
		}
		صف->type = BOX_جدول_صف;

		صف->والد = صف_گروہ;
		صف_گروہ->children = صف_گروہ->آخری = صف;

		گروہ_صف_گنتی = 1;

		/* Keep table's row count in sync */
		ستون_info->عدد_صفیں++;
	}

	صف_گروہ->صفیں = گروہ_صف_گنتی;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", صف_گروہ);
#اگر_ختم

	واپس true;
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
ساکن bool
box_normalise_جدول_spans(ساخت box *جدول,
			  ثابت ساخت box *جڑ,
			  ساخت وسعت_info *spans,
			  html_content *c)
{
	ساخت box *جدول_صف_گروہ;
	ساخت box *جدول_صف;
	ساخت box *جدول_cell;
	غیرعلامتی صحیح صفیں_بایاں = جدول->صفیں;
	غیرعلامتی صحیح گروہ_صفیں_بایاں;
	غیرعلامتی صحیح ستون;
	nscss_select_ctx ctx;

	ctx.جڑ_style = جڑ->style;

	/* Clear span data */
	memset(spans, 0, جدول->columns * حجم(ساخت وسعت_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	برائے (جدول_صف_گروہ = جدول->children;
	     جدول_صف_گروہ != تہی;
	     جدول_صف_گروہ = جدول_صف_گروہ->اگلا) {

		گروہ_صفیں_بایاں = جدول_صف_گروہ->صفیں;

		برائے (جدول_صف = جدول_صف_گروہ->children;
		     جدول_صف != تہی;
		     جدول_صف = جدول_صف->اگلا) {

			برائے (جدول_cell = جدول_صف->children;
			     جدول_cell != تہی;
			     جدول_cell = جدول_cell->اگلا) {

				/* colspan = 0 -> colspan = 1 */
				اگر (جدول_cell->columns == 0) {
					جدول_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				اگر (جدول_cell->صفیں == 0) {
					جدول_cell->صفیں = گروہ_صفیں_بایاں;
				}

				/* limit rowspans within group */
				اگر (جدول_cell->صفیں > گروہ_صفیں_بایاں) {
					جدول_cell->صفیں = گروہ_صفیں_بایاں;
				}

				/* Record span information */
				برائے (ستون = جدول_cell->آغاز_عمود;
						ستون < جدول_cell->آغاز_عمود +
						جدول_cell->columns; ستون++) {
					spans[ستون].صف_وسعت = جدول_cell->صفیں;
				}
			}

			/* Reduce span count of each column */
			برائے (ستون = 0; ستون < جدول->columns; ستون++) {
				اگر (spans[ستون].صف_وسعت == 0) {
					غیرعلامتی صحیح آغاز = ستون;
					css_computed_style *style;
					ساخت box *cell, *پچھلا;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(جدول_صف->style != تہی);

					/* Find width of gap */
					جب (ستون < جدول->columns &&
							spans[ستون].صف_وسعت ==
							0) {
						ستون++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_منوال_بھرا);
					ctx.بنیاد_url = c->بنیاد_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							جدول_صف->style);
					اگر (style == تہی)
						واپس false;

					cell = box_create(تہی, style, true,
							جدول_صف->href,
							جدول_صف->ہدف,
							تہی, تہی, c->bctx);
					اگر (cell == تہی) {
						css_computed_style_destroy(
								style);
						واپس false;
					}
					cell->type = BOX_جدول_CELL;

					cell->صفیں = 1;
					cell->columns = ستون - آغاز;
					cell->آغاز_عمود = آغاز;

					/* Find place to insert cell */
					برائے (پچھلا = جدول_صف->children;
							پچھلا != تہی;
							پچھلا = پچھلا->اگلا) {
						اگر (پچھلا->آغاز_عمود +
							پچھلا->columns ==
								آغاز)
							توڑو;
						اگر (پچھلا->اگلا == تہی)
							توڑو;
					}

					/* Insert it */
					اگر (پچھلا == تہی) {
						اگر (جدول_صف->children != تہی)
							جدول_صف->children->
								پچھلا = cell;
						ورنہ
							جدول_صف->آخری = cell;

						cell->اگلا =
							جدول_صف->children;
						جدول_صف->children = cell;
					} ورنہ {
						اگر (پچھلا->اگلا != تہی)
							پچھلا->اگلا->پچھلا = cell;
						ورنہ
							جدول_صف->آخری = cell;

						cell->اگلا = پچھلا->اگلا;
						پچھلا->اگلا = cell;
						cell->پچھلا = پچھلا;
					}
					cell->والد = جدول_صف;
				} ورنہ {
					spans[ستون].صف_وسعت--;
				}
			}

			أكد(صفیں_بایاں > 0);

			صفیں_بایاں--;
		}

		گروہ_صفیں_بایاں--;
	}

	واپس true;
}


ساکن bool
box_normalise_جدول(ساخت box *جدول, ثابت ساخت box *جڑ, html_content * c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;
	ساخت box *صف_گروہ;
	css_computed_style *style;
	ساخت columns ستون_info;
	nscss_select_ctx ctx;

	أكد(جدول != تہی);
	أكد(جدول->type == BOX_جدول);

	ctx.جڑ_style = جڑ->style;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", جدول);
#اگر_ختم

	ستون_info.عدد_columns = 1;
	ستون_info.current_عمود = 0;
	ستون_info.spans = حافظہ_دو(2 * حجم *ستون_info.spans);
	اگر (ستون_info.spans == تہی)
		واپس false;

	ستون_info.spans[0].صف_وسعت = ستون_info.spans[1].صف_وسعت = 0;
	ستون_info.spans[0].auto_صف = false;
	ستون_info.spans[1].auto_صف = false;
	ستون_info.عدد_صفیں = 0;

	برائے (بچہ = جدول->children; بچہ != تہی; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;
		انتخاب (بچہ->type) {
		حالت BOX_جدول_صف_گروہ:
			/* ok */
			اگر (box_normalise_جدول_صف_گروہ(بچہ, جڑ,
					&ستون_info, c) == false) {
				آزاد(ستون_info.spans);
				واپس false;
			}
			توڑو;
		حالت BOX_FLEX:
		حالت BOX_بلاک:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_جدول:
		حالت BOX_جدول_صف:
		حالت BOX_جدول_CELL:
			/* insert implied table row group */
			أكد(جدول->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					جدول->style);
			اگر (style == تہی) {
				آزاد(ستون_info.spans);
				واپس false;
			}

			صف_گروہ = box_create(تہی, style, true, جدول->href,
					جدول->ہدف, تہی, تہی, c->bctx);
			اگر (صف_گروہ == تہی) {
				css_computed_style_destroy(style);
				آزاد(ستون_info.spans);
				واپس false;
			}

			صف_گروہ->type = BOX_جدول_صف_گروہ;

			اگر (بچہ->پچھلا == تہی)
				جدول->children = صف_گروہ;
			ورنہ
				بچہ->پچھلا->اگلا = صف_گروہ;

			صف_گروہ->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(صف_گروہ, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			أكد(صف_گروہ->آخری != تہی);

			صف_گروہ->آخری->اگلا = تہی;
			صف_گروہ->اگلا = اگلا_بچہ = بچہ;
			اگر (صف_گروہ->اگلا != تہی)
				صف_گروہ->اگلا->پچھلا = صف_گروہ;
			ورنہ
				جدول->آخری = صف_گروہ;
			صف_گروہ->والد = جدول;

			اگر (box_normalise_جدول_صف_گروہ(صف_گروہ, جڑ,
					&ستون_info, c) == false) {
				آزاد(ستون_info.spans);
				واپس false;
			}
			توڑو;
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_بلاک:
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
		حالت BOX_BR:
		حالت BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			توڑو;
		طےشدہ:
			fprintf(stderr, "%i\n", بچہ->type);
			أكد(0);
		}
	}

	جدول->columns = ستون_info.عدد_columns;
	جدول->صفیں = ستون_info.عدد_صفیں;

	اگر (جدول->children == تہی) {
		ساخت box *صف;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#اگر_ختم

		أكد(جدول->style != تہی);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
		ctx.بنیاد_url = c->بنیاد_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				جدول->style);
		اگر (style == تہی) {
			آزاد(ستون_info.spans);
			واپس false;
		}

		صف_گروہ = box_create(تہی, style, true, جدول->href,
				جدول->ہدف, تہی, تہی, c->bctx);
		اگر (صف_گروہ == تہی) {
			css_computed_style_destroy(style);
			آزاد(ستون_info.spans);
			واپس false;
		}
		صف_گروہ->type = BOX_جدول_صف_گروہ;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروہ->style);
		اگر (style == تہی) {
			box_free(صف_گروہ);
			آزاد(ستون_info.spans);
			واپس false;
		}

		صف = box_create(تہی, style, true, صف_گروہ->href,
				صف_گروہ->ہدف, تہی, تہی, c->bctx);
		اگر (صف == تہی) {
			css_computed_style_destroy(style);
			box_free(صف_گروہ);
			آزاد(ستون_info.spans);
			واپس false;
		}
		صف->type = BOX_جدول_صف;

		صف->والد = صف_گروہ;
		صف_گروہ->children = صف_گروہ->آخری = صف;

		صف_گروہ->والد = جدول;
		جدول->children = جدول->آخری = صف_گروہ;

		جدول->صفیں = 1;
	}

	اگر (box_normalise_جدول_spans(جدول, جڑ, ستون_info.spans, c) == false) {
		آزاد(ستون_info.spans);
		واپس false;
	}

	آزاد(ستون_info.spans);

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", جدول);
#اگر_ختم

	واپس true;
}

ساکن bool box_normalise_flex(
		ساخت box *flex_container,
		ثابت ساخت box *جڑ,
		html_content *c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;
	ساخت box *implied_flex_شے;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != تہی);
	أكد(جڑ != تہی);

	ctx.جڑ_style = جڑ->style;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#اگر_ختم

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	برائے (بچہ = flex_container->children; بچہ != تہی; بچہ = اگلا_بچہ) {
#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				بچہ, بچہ->type);
#اگر_ختم

		اگلا_بچہ = بچہ->اگلا;	/* child may be destroyed */

		انتخاب (بچہ->type) {
		حالت BOX_FLEX:
			/* ok */
			اگر (box_normalise_flex(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_بلاک:
			/* ok */
			اگر (box_normalise_بلاک(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			اگر (style == تہی)
				واپس false;

			implied_flex_شے = box_create(تہی, style, true,
					flex_container->href,
					flex_container->ہدف,
					تہی, تہی, c->bctx);
			اگر (implied_flex_شے == تہی) {
				css_computed_style_destroy(style);
				واپس false;
			}
			implied_flex_شے->type = BOX_بلاک;

			اگر (بچہ->پچھلا == تہی)
				flex_container->children = implied_flex_شے;
			ورنہ
				بچہ->پچھلا->اگلا = implied_flex_شے;

			implied_flex_شے->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی &&
					بچہ->type == BOX_INLINE_CONTAINER) {
				box_جوڑو_بچہ(implied_flex_شے, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			implied_flex_شے->آخری->اگلا = تہی;
			implied_flex_شے->اگلا = اگلا_بچہ = بچہ;
			اگر (implied_flex_شے->اگلا != تہی)
				implied_flex_شے->اگلا->پچھلا = implied_flex_شے;
			ورنہ
				flex_container->آخری = implied_flex_شے;
			implied_flex_شے->والد = flex_container;

			اگر (box_normalise_بلاک(implied_flex_شے,
					جڑ, c) == false)
				واپس false;
			توڑو;

		حالت BOX_جدول:
			اگر (box_normalise_جدول(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_بلاک:
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
		حالت BOX_BR:
		حالت BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			توڑو;
		حالت BOX_جدول_صف_گروہ:
		حالت BOX_جدول_صف:
		حالت BOX_جدول_CELL:
			/* insert implied table */
			أكد(flex_container->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			اگر (style == تہی)
				واپس false;

			implied_flex_شے = box_create(تہی, style, true,
					flex_container->href,
					flex_container->ہدف,
					تہی, تہی, c->bctx);
			اگر (implied_flex_شے == تہی) {
				css_computed_style_destroy(style);
				واپس false;
			}
			implied_flex_شے->type = BOX_جدول;

			اگر (بچہ->پچھلا == تہی)
				flex_container->children = implied_flex_شے;
			ورنہ
				بچہ->پچھلا->اگلا = implied_flex_شے;

			implied_flex_شے->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی && (
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(implied_flex_شے, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			implied_flex_شے->آخری->اگلا = تہی;
			implied_flex_شے->اگلا = اگلا_بچہ = بچہ;
			اگر (implied_flex_شے->اگلا != تہی)
				implied_flex_شے->اگلا->پچھلا = implied_flex_شے;
			ورنہ
				flex_container->آخری = implied_flex_شے;
			implied_flex_شے->والد = flex_container;

			اگر (box_normalise_جدول(implied_flex_شے,
					جڑ, c) == false)
				واپس false;
			توڑو;
		طےشدہ:
			أكد(0);
		}
	}

	واپس true;
}

ساکن bool
box_normalise_inline_container(ساخت box *cont,
			       ثابت ساخت box *جڑ,
			       html_content * c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;

	أكد(cont != تہی);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#اگر_ختم

	برائے (بچہ = cont->children; بچہ != تہی; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;
		انتخاب (بچہ->type) {
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_BR:
		حالت BOX_متن:
			/* ok */
			توڑو;
		حالت BOX_INLINE_بلاک:
			/* ok */
			اگر (box_normalise_بلاک(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE_FLEX:
			/* ok */
			اگر (box_normalise_flex(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
			/* ok */
			أكد(بچہ->children != تہی);

			انتخاب (بچہ->children->type) {
			حالت BOX_بلاک:
				اگر (box_normalise_بلاک(بچہ->children, جڑ,
						c) == false)
					واپس false;
				توڑو;
			حالت BOX_جدول:
				اگر (box_normalise_جدول(بچہ->children, جڑ,
						c) == false)
					واپس false;
				توڑو;
			حالت BOX_FLEX:
				اگر (box_normalise_flex(بچہ->children, جڑ,
						c) == false)
					واپس false;
				توڑو;
			طےشدہ:
				أكد(0);
			}

			اگر (بچہ->children == تہی) {
				/* the child has destroyed itself: remove float */
				اگر (بچہ->پچھلا == تہی)
					بچہ->والد->children = بچہ->اگلا;
				ورنہ
					بچہ->پچھلا->اگلا = بچہ->اگلا;
				اگر (بچہ->اگلا != تہی)
					بچہ->اگلا->پچھلا = بچہ->پچھلا;
				ورنہ
					بچہ->والد->آخری = بچہ->پچھلا;

				box_free(بچہ);
			}
			توڑو;
		حالت BOX_FLEX:
		حالت BOX_بلاک:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_جدول:
		حالت BOX_جدول_صف_گروہ:
		حالت BOX_جدول_صف:
		حالت BOX_جدول_CELL:
		طےشدہ:
			أكد(0);
		}
	}

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#اگر_ختم

	واپس true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_بلاک(ساخت box *بلاک, ثابت ساخت box *جڑ, html_content *c)
{
	ساخت box *بچہ;
	ساخت box *اگلا_بچہ;
	ساخت box *جدول;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(بلاک != تہی);
	أكد(جڑ != تہی);

	ctx.جڑ_style = جڑ->style;

#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", بلاک, بلاک->type);
#اگر_ختم

	أكد(بلاک->type == BOX_بلاک || بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	برائے (بچہ = بلاک->children; بچہ != تہی; بچہ = اگلا_بچہ) {
#اگر_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", بچہ,
		      بچہ->type);
#اگر_ختم

		اگلا_بچہ = بچہ->اگلا;	/* child may be destroyed */

		انتخاب (بچہ->type) {
		حالت BOX_FLEX:
			/* ok */
			اگر (box_normalise_flex(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_بلاک:
			/* ok */
			اگر (box_normalise_بلاک(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE_CONTAINER:
			اگر (box_normalise_inline_container(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_جدول:
			اگر (box_normalise_جدول(بچہ, جڑ, c) == false)
				واپس false;
			توڑو;
		حالت BOX_INLINE:
		حالت BOX_INLINE_انتہا:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_بلاک:
		حالت BOX_FLOAT_بایاں:
		حالت BOX_FLOAT_دایاں:
		حالت BOX_BR:
		حالت BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			توڑو;
		حالت BOX_جدول_صف_گروہ:
		حالت BOX_جدول_صف:
		حالت BOX_جدول_CELL:
			/* insert implied table */
			أكد(بلاک->style != تہی);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا);
			ctx.بنیاد_url = c->بنیاد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					بلاک->style);
			اگر (style == تہی)
				واپس false;

			جدول = box_create(تہی, style, true, بلاک->href,
					بلاک->ہدف, تہی, تہی, c->bctx);
			اگر (جدول == تہی) {
				css_computed_style_destroy(style);
				واپس false;
			}
			جدول->type = BOX_جدول;

			اگر (بچہ->پچھلا == تہی)
				بلاک->children = جدول;
			ورنہ
				بچہ->پچھلا->اگلا = جدول;

			جدول->پچھلا = بچہ->پچھلا;

			جب (بچہ != تہی && (
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(جدول, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = تہی;
				بچہ = اگلا_بچہ;
			}

			جدول->آخری->اگلا = تہی;
			جدول->اگلا = اگلا_بچہ = بچہ;
			اگر (جدول->اگلا != تہی)
				جدول->اگلا->پچھلا = جدول;
			ورنہ
				بلاک->آخری = جدول;
			جدول->والد = بلاک;

			اگر (box_normalise_جدول(جدول, جڑ, c) == false)
				واپس false;
			توڑو;
		طےشدہ:
			أكد(0);
		}
	}

	واپس true;
}
