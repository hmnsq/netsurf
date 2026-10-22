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
#تعریف_رد BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
بنية وسعت_info {
	/** Number of rows this cell spans */
	بنانشانی صحيح صف_وسعت;
	/** Row group of cell */
	بنية box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_صف;
};

/**
 * Column record for a table
 */
بنية columns {
	/** Current column index */
	بنانشانی صحيح current_کالم;
	/** Number of columns in main part of table 1..max columns */
	بنانشانی صحيح عدد_columns;
	/** Information about columns in main table, array [0, num_columns) */
	بنية وسعت_info *spans;
	/** Number of rows in table */
	بنانشانی صحيح عدد_صفاں;
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
calculate_جدول_صف(بنية columns *خانہ_info,
		    بنانشانی صحيح خانہ_وسعت,
		    بنانشانی صحيح صف_وسعت,
		    بنانشانی صحيح *شروع_کالم,
		    بنية box *cell)
{
	بنانشانی صحيح cell_شروع_خانہ = خانہ_info->current_کالم;
	بنانشانی صحيح cell_سرا_خانہ;
	بنانشانی صحيح i;
	بنية وسعت_info *spans;
	بنية box *rg = cell->ماپا->ماپا; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	طالما (خانہ_info->spans[cell_شروع_خانہ].صف_وسعت != 0 &&
			خانہ_info->spans[cell_شروع_خانہ].rg == rg) {
		cell_شروع_خانہ++;
	}

	/* Update current column with calculated start */
	خانہ_info->current_کالم = cell_شروع_خانہ;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	إذا (خانہ_وسعت == 0)
		خانہ_وسعت = 1;

	cell_سرا_خانہ = cell_شروع_خانہ + خانہ_وسعت;

	إذا (خانہ_info->عدد_columns < cell_سرا_خانہ) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(خانہ_info->spans,
				حجم *spans * (cell_سرا_خانہ + 1));
		إذا (spans == عدم)
			أرجع false;

		خانہ_info->spans = spans;
		خانہ_info->عدد_columns = cell_سرا_خانہ;

		/* Mark new final column as sentinel */
		خانہ_info->spans[cell_سرا_خانہ].صف_وسعت = 0;
		خانہ_info->spans[cell_سرا_خانہ].auto_صف = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	لكل (i = cell_شروع_خانہ; i < cell_سرا_خانہ; i++) {
		خانہ_info->spans[i].صف_وسعت = (صف_وسعت == 0) ? 1 : صف_وسعت;
		خانہ_info->spans[i].auto_صف = (صف_وسعت == 0);
		خانہ_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	خانہ_info->current_کالم = cell_سرا_خانہ;

	*شروع_کالم = cell_شروع_خانہ;

	أرجع true;
}


ساكن bool
box_normalise_جدول_صف(بنية box *صف,
			ثابت بنية box *جڑ,
			بنية columns *خانہ_info,
			html_content * c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;
	بنية box *cell = عدم;
	css_computed_style *style;
	بنانشانی صحيح i;
	nscss_select_ctx ctx;

	أكد(صف != عدم);
	أكد(صف->type == BOX_جدول_صف);

	ctx.جڑ_style = جڑ->style;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", صف);
#جے_انت

	لكل (بچہ = صف->children; بچہ != عدم; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;

		بدل (بچہ->type) {
		حالة BOX_جدول_CELL:
			/* ok */
			إذا (box_normalise_بلاک(بچہ, جڑ, c) == false)
				أرجع false;
			cell = بچہ;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروہ:
		حالة BOX_جدول_صف:
			/* insert implied table cell */
			أكد(صف->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف->style);
			إذا (style == عدم)
				أرجع false;

			cell = box_create(عدم, style, true, صف->href,
					صف->نشانہ, عدم, عدم, c->bctx);
			إذا (cell == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			cell->type = BOX_جدول_CELL;

			إذا (بچہ->prev == عدم)
				صف->children = cell;
			وإلا
				بچہ->prev->اگلا = cell;

			cell->prev = بچہ->prev;

			طالما (بچہ != عدم && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف)) {
				box_جوڑو_بچہ(cell, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			أكد(cell->آخری != عدم);

			cell->آخری->اگلا = عدم;
			cell->اگلا = اگلا_بچہ = بچہ;
			إذا (cell->اگلا != عدم)
				cell->اگلا->prev = cell;
			وإلا
				صف->آخری = cell;
			cell->ماپا = صف;

			إذا (box_normalise_بلاک(cell, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}

		إذا (calculate_جدول_صف(خانہ_info, cell->columns, cell->صفاں,
				&cell->شروع_کالم, cell) == false)
			أرجع false;
	}


	/* Update row spanning details for all columns */
	لكل (i = 0; i < خانہ_info->عدد_columns; i++) {
		إذا (خانہ_info->spans[i].صف_وسعت != 0 &&
				خانہ_info->spans[i].auto_صف == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			خانہ_info->spans[i].صف_وسعت--;
		}
	}

	/* Reset current column for next row */
	خانہ_info->current_کالم = 0;

	/* Increment row counter */
	خانہ_info->عدد_صفاں++;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", صف);
#جے_انت

	أرجع true;
}


ساكن bool
box_normalise_جدول_صف_گروہ(بنية box *صف_گروہ,
			      ثابت بنية box *جڑ,
			      بنية columns *خانہ_info,
			      html_content * c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;
	بنية box *صف;
	css_computed_style *style;
	nscss_select_ctx ctx;
	بنانشانی صحيح گروہ_صف_گنتی = 0;

	أكد(صف_گروہ != 0);
	أكد(صف_گروہ->type == BOX_جدول_صف_گروہ);

	ctx.جڑ_style = جڑ->style;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", صف_گروہ);
#جے_انت

	لكل (بچہ = صف_گروہ->children; بچہ != عدم; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;

		بدل (بچہ->type) {
		حالة BOX_جدول_صف:
			/* ok */
			گروہ_صف_گنتی++;
			إذا (box_normalise_جدول_صف(بچہ, جڑ, خانہ_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروہ:
		حالة BOX_جدول_CELL:
			/* insert implied table row */
			أكد(صف_گروہ->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف_گروہ->style);
			إذا (style == عدم)
				أرجع false;

			صف = box_create(عدم, style, true, صف_گروہ->href,
					صف_گروہ->نشانہ, عدم, عدم, c->bctx);
			إذا (صف == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			صف->type = BOX_جدول_صف;

			إذا (بچہ->prev == عدم)
				صف_گروہ->children = صف;
			وإلا
				بچہ->prev->اگلا = صف;

			صف->prev = بچہ->prev;

			طالما (بچہ != عدم && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(صف, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			أكد(صف->آخری != عدم);

			صف->آخری->اگلا = عدم;
			صف->اگلا = اگلا_بچہ = بچہ;
			إذا (صف->اگلا != عدم)
				صف->اگلا->prev = صف;
			وإلا
				صف_گروہ->آخری = صف;
			صف->ماپا = صف_گروہ;

			گروہ_صف_گنتی++;
			إذا (box_normalise_جدول_صف(صف, جڑ, خانہ_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	إذا (صف_گروہ->children == عدم) {
#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#جے_انت

		أكد(صف_گروہ->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
		ctx.ادھار_url = c->ادھار_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروہ->style);
		إذا (style == عدم) {
			أرجع false;
		}

		صف = box_create(عدم, style, true, صف_گروہ->href,
				صف_گروہ->نشانہ, عدم, عدم, c->bctx);
		إذا (صف == عدم) {
			css_computed_style_destroy(style);
			أرجع false;
		}
		صف->type = BOX_جدول_صف;

		صف->ماپا = صف_گروہ;
		صف_گروہ->children = صف_گروہ->آخری = صف;

		گروہ_صف_گنتی = 1;

		/* Keep table's row count in sync */
		خانہ_info->عدد_صفاں++;
	}

	صف_گروہ->صفاں = گروہ_صف_گنتی;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", صف_گروہ);
#جے_انت

	أرجع true;
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
box_normalise_جدول_spans(بنية box *جدول,
			  ثابت بنية box *جڑ,
			  بنية وسعت_info *spans,
			  html_content *c)
{
	بنية box *جدول_صف_گروہ;
	بنية box *جدول_صف;
	بنية box *جدول_cell;
	بنانشانی صحيح صفاں_بایاں = جدول->صفاں;
	بنانشانی صحيح گروہ_صفاں_بایاں;
	بنانشانی صحيح خانہ;
	nscss_select_ctx ctx;

	ctx.جڑ_style = جڑ->style;

	/* Clear span data */
	memset(spans, 0, جدول->columns * حجم(بنية وسعت_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	لكل (جدول_صف_گروہ = جدول->children;
	     جدول_صف_گروہ != عدم;
	     جدول_صف_گروہ = جدول_صف_گروہ->اگلا) {

		گروہ_صفاں_بایاں = جدول_صف_گروہ->صفاں;

		لكل (جدول_صف = جدول_صف_گروہ->children;
		     جدول_صف != عدم;
		     جدول_صف = جدول_صف->اگلا) {

			لكل (جدول_cell = جدول_صف->children;
			     جدول_cell != عدم;
			     جدول_cell = جدول_cell->اگلا) {

				/* colspan = 0 -> colspan = 1 */
				إذا (جدول_cell->columns == 0) {
					جدول_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				إذا (جدول_cell->صفاں == 0) {
					جدول_cell->صفاں = گروہ_صفاں_بایاں;
				}

				/* limit rowspans within group */
				إذا (جدول_cell->صفاں > گروہ_صفاں_بایاں) {
					جدول_cell->صفاں = گروہ_صفاں_بایاں;
				}

				/* Record span information */
				لكل (خانہ = جدول_cell->شروع_کالم;
						خانہ < جدول_cell->شروع_کالم +
						جدول_cell->columns; خانہ++) {
					spans[خانہ].صف_وسعت = جدول_cell->صفاں;
				}
			}

			/* Reduce span count of each column */
			لكل (خانہ = 0; خانہ < جدول->columns; خانہ++) {
				إذا (spans[خانہ].صف_وسعت == 0) {
					بنانشانی صحيح شروع = خانہ;
					css_computed_style *style;
					بنية box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(جدول_صف->style != عدم);

					/* Find width of gap */
					طالما (خانہ < جدول->columns &&
							spans[خانہ].صف_وسعت ==
							0) {
						خانہ++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_منوال_بھریا);
					ctx.ادھار_url = c->ادھار_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							جدول_صف->style);
					إذا (style == عدم)
						أرجع false;

					cell = box_create(عدم, style, true,
							جدول_صف->href,
							جدول_صف->نشانہ,
							عدم, عدم, c->bctx);
					إذا (cell == عدم) {
						css_computed_style_destroy(
								style);
						أرجع false;
					}
					cell->type = BOX_جدول_CELL;

					cell->صفاں = 1;
					cell->columns = خانہ - شروع;
					cell->شروع_کالم = شروع;

					/* Find place to insert cell */
					لكل (prev = جدول_صف->children;
							prev != عدم;
							prev = prev->اگلا) {
						إذا (prev->شروع_کالم +
							prev->columns ==
								شروع)
							اكسر;
						إذا (prev->اگلا == عدم)
							اكسر;
					}

					/* Insert it */
					إذا (prev == عدم) {
						إذا (جدول_صف->children != عدم)
							جدول_صف->children->
								prev = cell;
						وإلا
							جدول_صف->آخری = cell;

						cell->اگلا =
							جدول_صف->children;
						جدول_صف->children = cell;
					} وإلا {
						إذا (prev->اگلا != عدم)
							prev->اگلا->prev = cell;
						وإلا
							جدول_صف->آخری = cell;

						cell->اگلا = prev->اگلا;
						prev->اگلا = cell;
						cell->prev = prev;
					}
					cell->ماپا = جدول_صف;
				} وإلا {
					spans[خانہ].صف_وسعت--;
				}
			}

			أكد(صفاں_بایاں > 0);

			صفاں_بایاں--;
		}

		گروہ_صفاں_بایاں--;
	}

	أرجع true;
}


ساكن bool
box_normalise_جدول(بنية box *جدول, ثابت بنية box *جڑ, html_content * c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;
	بنية box *صف_گروہ;
	css_computed_style *style;
	بنية columns خانہ_info;
	nscss_select_ctx ctx;

	أكد(جدول != عدم);
	أكد(جدول->type == BOX_جدول);

	ctx.جڑ_style = جڑ->style;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", جدول);
#جے_انت

	خانہ_info.عدد_columns = 1;
	خانہ_info.current_کالم = 0;
	خانہ_info.spans = حافظہ_دیو(2 * حجم *خانہ_info.spans);
	إذا (خانہ_info.spans == عدم)
		أرجع false;

	خانہ_info.spans[0].صف_وسعت = خانہ_info.spans[1].صف_وسعت = 0;
	خانہ_info.spans[0].auto_صف = false;
	خانہ_info.spans[1].auto_صف = false;
	خانہ_info.عدد_صفاں = 0;

	لكل (بچہ = جدول->children; بچہ != عدم; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;
		بدل (بچہ->type) {
		حالة BOX_جدول_صف_گروہ:
			/* ok */
			إذا (box_normalise_جدول_صف_گروہ(بچہ, جڑ,
					&خانہ_info, c) == false) {
				حرر(خانہ_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table row group */
			أكد(جدول->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					جدول->style);
			إذا (style == عدم) {
				حرر(خانہ_info.spans);
				أرجع false;
			}

			صف_گروہ = box_create(عدم, style, true, جدول->href,
					جدول->نشانہ, عدم, عدم, c->bctx);
			إذا (صف_گروہ == عدم) {
				css_computed_style_destroy(style);
				حرر(خانہ_info.spans);
				أرجع false;
			}

			صف_گروہ->type = BOX_جدول_صف_گروہ;

			إذا (بچہ->prev == عدم)
				جدول->children = صف_گروہ;
			وإلا
				بچہ->prev->اگلا = صف_گروہ;

			صف_گروہ->prev = بچہ->prev;

			طالما (بچہ != عدم && (
					بچہ->type == BOX_FLEX ||
					بچہ->type == BOX_بلاک ||
					بچہ->type == BOX_INLINE_CONTAINER ||
					بچہ->type == BOX_جدول ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(صف_گروہ, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			أكد(صف_گروہ->آخری != عدم);

			صف_گروہ->آخری->اگلا = عدم;
			صف_گروہ->اگلا = اگلا_بچہ = بچہ;
			إذا (صف_گروہ->اگلا != عدم)
				صف_گروہ->اگلا->prev = صف_گروہ;
			وإلا
				جدول->آخری = صف_گروہ;
			صف_گروہ->ماپا = جدول;

			إذا (box_normalise_جدول_صف_گروہ(صف_گروہ, جڑ,
					&خانہ_info, c) == false) {
				حرر(خانہ_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			fprintf(stderr, "%i\n", بچہ->type);
			أكد(0);
		}
	}

	جدول->columns = خانہ_info.عدد_columns;
	جدول->صفاں = خانہ_info.عدد_صفاں;

	إذا (جدول->children == عدم) {
		بنية box *صف;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#جے_انت

		أكد(جدول->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
		ctx.ادھار_url = c->ادھار_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				جدول->style);
		إذا (style == عدم) {
			حرر(خانہ_info.spans);
			أرجع false;
		}

		صف_گروہ = box_create(عدم, style, true, جدول->href,
				جدول->نشانہ, عدم, عدم, c->bctx);
		إذا (صف_گروہ == عدم) {
			css_computed_style_destroy(style);
			حرر(خانہ_info.spans);
			أرجع false;
		}
		صف_گروہ->type = BOX_جدول_صف_گروہ;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروہ->style);
		إذا (style == عدم) {
			box_free(صف_گروہ);
			حرر(خانہ_info.spans);
			أرجع false;
		}

		صف = box_create(عدم, style, true, صف_گروہ->href,
				صف_گروہ->نشانہ, عدم, عدم, c->bctx);
		إذا (صف == عدم) {
			css_computed_style_destroy(style);
			box_free(صف_گروہ);
			حرر(خانہ_info.spans);
			أرجع false;
		}
		صف->type = BOX_جدول_صف;

		صف->ماپا = صف_گروہ;
		صف_گروہ->children = صف_گروہ->آخری = صف;

		صف_گروہ->ماپا = جدول;
		جدول->children = جدول->آخری = صف_گروہ;

		جدول->صفاں = 1;
	}

	إذا (box_normalise_جدول_spans(جدول, جڑ, خانہ_info.spans, c) == false) {
		حرر(خانہ_info.spans);
		أرجع false;
	}

	حرر(خانہ_info.spans);

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", جدول);
#جے_انت

	أرجع true;
}

ساكن bool box_normalise_flex(
		بنية box *flex_container,
		ثابت بنية box *جڑ,
		html_content *c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;
	بنية box *implied_flex_شے;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != عدم);
	أكد(جڑ != عدم);

	ctx.جڑ_style = جڑ->style;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#جے_انت

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	لكل (بچہ = flex_container->children; بچہ != عدم; بچہ = اگلا_بچہ) {
#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				بچہ, بچہ->type);
#جے_انت

		اگلا_بچہ = بچہ->اگلا;	/* child may be destroyed */

		بدل (بچہ->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_شے = box_create(عدم, style, true,
					flex_container->href,
					flex_container->نشانہ,
					عدم, عدم, c->bctx);
			إذا (implied_flex_شے == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_شے->type = BOX_بلاک;

			إذا (بچہ->prev == عدم)
				flex_container->children = implied_flex_شے;
			وإلا
				بچہ->prev->اگلا = implied_flex_شے;

			implied_flex_شے->prev = بچہ->prev;

			طالما (بچہ != عدم &&
					بچہ->type == BOX_INLINE_CONTAINER) {
				box_جوڑو_بچہ(implied_flex_شے, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			implied_flex_شے->آخری->اگلا = عدم;
			implied_flex_شے->اگلا = اگلا_بچہ = بچہ;
			إذا (implied_flex_شے->اگلا != عدم)
				implied_flex_شے->اگلا->prev = implied_flex_شے;
			وإلا
				flex_container->آخری = implied_flex_شے;
			implied_flex_شے->ماپا = flex_container;

			إذا (box_normalise_بلاک(implied_flex_شے,
					جڑ, c) == false)
				أرجع false;
			اكسر;

		حالة BOX_جدول:
			إذا (box_normalise_جدول(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_صف_گروہ:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_شے = box_create(عدم, style, true,
					flex_container->href,
					flex_container->نشانہ,
					عدم, عدم, c->bctx);
			إذا (implied_flex_شے == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_شے->type = BOX_جدول;

			إذا (بچہ->prev == عدم)
				flex_container->children = implied_flex_شے;
			وإلا
				بچہ->prev->اگلا = implied_flex_شے;

			implied_flex_شے->prev = بچہ->prev;

			طالما (بچہ != عدم && (
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(implied_flex_شے, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			implied_flex_شے->آخری->اگلا = عدم;
			implied_flex_شے->اگلا = اگلا_بچہ = بچہ;
			إذا (implied_flex_شے->اگلا != عدم)
				implied_flex_شے->اگلا->prev = implied_flex_شے;
			وإلا
				flex_container->آخری = implied_flex_شے;
			implied_flex_شے->ماپا = flex_container;

			إذا (box_normalise_جدول(implied_flex_شے,
					جڑ, c) == false)
				أرجع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	أرجع true;
}

ساكن bool
box_normalise_inline_container(بنية box *cont,
			       ثابت بنية box *جڑ,
			       html_content * c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;

	أكد(cont != عدم);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#جے_انت

	لكل (بچہ = cont->children; بچہ != عدم; بچہ = اگلا_بچہ) {
		اگلا_بچہ = بچہ->اگلا;
		بدل (بچہ->type) {
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_BR:
		حالة BOX_متن:
			/* ok */
			اكسر;
		حالة BOX_INLINE_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_FLEX:
			/* ok */
			إذا (box_normalise_flex(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
			/* ok */
			أكد(بچہ->children != عدم);

			بدل (بچہ->children->type) {
			حالة BOX_بلاک:
				إذا (box_normalise_بلاک(بچہ->children, جڑ,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_جدول:
				إذا (box_normalise_جدول(بچہ->children, جڑ,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_FLEX:
				إذا (box_normalise_flex(بچہ->children, جڑ,
						c) == false)
					أرجع false;
				اكسر;
			افتراضي:
				أكد(0);
			}

			إذا (بچہ->children == عدم) {
				/* the child has destroyed itself: remove float */
				إذا (بچہ->prev == عدم)
					بچہ->ماپا->children = بچہ->اگلا;
				وإلا
					بچہ->prev->اگلا = بچہ->اگلا;
				إذا (بچہ->اگلا != عدم)
					بچہ->اگلا->prev = بچہ->prev;
				وإلا
					بچہ->ماپا->آخری = بچہ->prev;

				box_free(بچہ);
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروہ:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
		افتراضي:
			أكد(0);
		}
	}

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#جے_انت

	أرجع true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_بلاک(بنية box *بلاک, ثابت بنية box *جڑ, html_content *c)
{
	بنية box *بچہ;
	بنية box *اگلا_بچہ;
	بنية box *جدول;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(بلاک != عدم);
	أكد(جڑ != عدم);

	ctx.جڑ_style = جڑ->style;

#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", بلاک, بلاک->type);
#جے_انت

	أكد(بلاک->type == BOX_بلاک || بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	لكل (بچہ = بلاک->children; بچہ != عدم; بچہ = اگلا_بچہ) {
#جے_تعریف_شدہ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", بچہ,
		      بچہ->type);
#جے_انت

		اگلا_بچہ = بچہ->اگلا;	/* child may be destroyed */

		بدل (بچہ->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			إذا (box_normalise_inline_container(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_جدول:
			إذا (box_normalise_جدول(بچہ, جڑ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_سرا:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_بایاں:
		حالة BOX_FLOAT_دایاں:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_صف_گروہ:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(بلاک->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھریا);
			ctx.ادھار_url = c->ادھار_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					بلاک->style);
			إذا (style == عدم)
				أرجع false;

			جدول = box_create(عدم, style, true, بلاک->href,
					بلاک->نشانہ, عدم, عدم, c->bctx);
			إذا (جدول == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			جدول->type = BOX_جدول;

			إذا (بچہ->prev == عدم)
				بلاک->children = جدول;
			وإلا
				بچہ->prev->اگلا = جدول;

			جدول->prev = بچہ->prev;

			طالما (بچہ != عدم && (
					بچہ->type == BOX_جدول_صف_گروہ ||
					بچہ->type == BOX_جدول_صف ||
					بچہ->type == BOX_جدول_CELL)) {
				box_جوڑو_بچہ(جدول, بچہ);

				اگلا_بچہ = بچہ->اگلا;
				بچہ->اگلا = عدم;
				بچہ = اگلا_بچہ;
			}

			جدول->آخری->اگلا = عدم;
			جدول->اگلا = اگلا_بچہ = بچہ;
			إذا (جدول->اگلا != عدم)
				جدول->اگلا->prev = جدول;
			وإلا
				بلاک->آخری = جدول;
			جدول->ماپا = بلاک;

			إذا (box_normalise_جدول(جدول, جڑ, c) == false)
				أرجع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	أرجع true;
}
