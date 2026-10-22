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
#لغو_تعریف BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
بنية بازه_info {
	/** Number of rows this cell spans */
	بیعلامت صحيح ردیف_بازه;
	/** Row group of cell */
	بنية box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_ردیف;
};

/**
 * Column record for a table
 */
بنية columns {
	/** Current column index */
	بیعلامت صحيح current_ستونی;
	/** Number of columns in main part of table 1..max columns */
	بیعلامت صحيح رقم_columns;
	/** Information about columns in main table, array [0, num_columns) */
	بنية بازه_info *spans;
	/** Number of rows in table */
	بیعلامت صحيح رقم_ردیفها;
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
calculate_جدول_ردیف(بنية columns *ستون_info,
		    بیعلامت صحيح ستون_بازه,
		    بیعلامت صحيح ردیف_بازه,
		    بیعلامت صحيح *شروع_ستونی,
		    بنية box *cell)
{
	بیعلامت صحيح cell_شروع_ستون = ستون_info->current_ستونی;
	بیعلامت صحيح cell_انتها_ستون;
	بیعلامت صحيح i;
	بنية بازه_info *spans;
	بنية box *rg = cell->والد->والد; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	طالما (ستون_info->spans[cell_شروع_ستون].ردیف_بازه != 0 &&
			ستون_info->spans[cell_شروع_ستون].rg == rg) {
		cell_شروع_ستون++;
	}

	/* Update current column with calculated start */
	ستون_info->current_ستونی = cell_شروع_ستون;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	إذا (ستون_بازه == 0)
		ستون_بازه = 1;

	cell_انتها_ستون = cell_شروع_ستون + ستون_بازه;

	إذا (ستون_info->رقم_columns < cell_انتها_ستون) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(ستون_info->spans,
				حجم *spans * (cell_انتها_ستون + 1));
		إذا (spans == عدم)
			أرجع false;

		ستون_info->spans = spans;
		ستون_info->رقم_columns = cell_انتها_ستون;

		/* Mark new final column as sentinel */
		ستون_info->spans[cell_انتها_ستون].ردیف_بازه = 0;
		ستون_info->spans[cell_انتها_ستون].auto_ردیف = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	لكل (i = cell_شروع_ستون; i < cell_انتها_ستون; i++) {
		ستون_info->spans[i].ردیف_بازه = (ردیف_بازه == 0) ? 1 : ردیف_بازه;
		ستون_info->spans[i].auto_ردیف = (ردیف_بازه == 0);
		ستون_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	ستون_info->current_ستونی = cell_انتها_ستون;

	*شروع_ستونی = cell_شروع_ستون;

	أرجع true;
}


ساكن bool
box_normalise_جدول_ردیف(بنية box *ردیف,
			ثابت بنية box *ریشه,
			بنية columns *ستون_info,
			html_content * c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;
	بنية box *cell = عدم;
	css_computed_style *style;
	بیعلامت صحيح i;
	nscss_select_ctx ctx;

	أكد(ردیف != عدم);
	أكد(ردیف->type == BOX_جدول_ردیف);

	ctx.ریشه_style = ریشه->style;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", ردیف);
#پایان_اگر

	لكل (فرزند = ردیف->children; فرزند != عدم; فرزند = بعدی_فرزند) {
		بعدی_فرزند = فرزند->بعدی;

		بدل (فرزند->type) {
		حالة BOX_جدول_CELL:
			/* ok */
			إذا (box_normalise_بلوک(فرزند, ریشه, c) == false)
				أرجع false;
			cell = فرزند;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلوک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_ردیف_گروه:
		حالة BOX_جدول_ردیف:
			/* insert implied table cell */
			أكد(ردیف->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ردیف->style);
			إذا (style == عدم)
				أرجع false;

			cell = box_create(عدم, style, true, ردیف->href,
					ردیف->هدف, عدم, عدم, c->bctx);
			إذا (cell == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			cell->type = BOX_جدول_CELL;

			إذا (فرزند->prev == عدم)
				ردیف->children = cell;
			وإلا
				فرزند->prev->بعدی = cell;

			cell->prev = فرزند->prev;

			طالما (فرزند != عدم && (
					فرزند->type == BOX_FLEX ||
					فرزند->type == BOX_بلوک ||
					فرزند->type == BOX_INLINE_CONTAINER ||
					فرزند->type == BOX_جدول ||
					فرزند->type == BOX_جدول_ردیف_گروه ||
					فرزند->type == BOX_جدول_ردیف)) {
				box_جمع_کن_فرزند(cell, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			أكد(cell->آخرین != عدم);

			cell->آخرین->بعدی = عدم;
			cell->بعدی = بعدی_فرزند = فرزند;
			إذا (cell->بعدی != عدم)
				cell->بعدی->prev = cell;
			وإلا
				ردیف->آخرین = cell;
			cell->والد = ردیف;

			إذا (box_normalise_بلوک(cell, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلوک:
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}

		إذا (calculate_جدول_ردیف(ستون_info, cell->columns, cell->ردیفها,
				&cell->شروع_ستونی, cell) == false)
			أرجع false;
	}


	/* Update row spanning details for all columns */
	لكل (i = 0; i < ستون_info->رقم_columns; i++) {
		إذا (ستون_info->spans[i].ردیف_بازه != 0 &&
				ستون_info->spans[i].auto_ردیف == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			ستون_info->spans[i].ردیف_بازه--;
		}
	}

	/* Reset current column for next row */
	ستون_info->current_ستونی = 0;

	/* Increment row counter */
	ستون_info->رقم_ردیفها++;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", ردیف);
#پایان_اگر

	أرجع true;
}


ساكن bool
box_normalise_جدول_ردیف_گروه(بنية box *ردیف_گروه,
			      ثابت بنية box *ریشه,
			      بنية columns *ستون_info,
			      html_content * c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;
	بنية box *ردیف;
	css_computed_style *style;
	nscss_select_ctx ctx;
	بیعلامت صحيح گروه_ردیف_شمار = 0;

	أكد(ردیف_گروه != 0);
	أكد(ردیف_گروه->type == BOX_جدول_ردیف_گروه);

	ctx.ریشه_style = ریشه->style;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", ردیف_گروه);
#پایان_اگر

	لكل (فرزند = ردیف_گروه->children; فرزند != عدم; فرزند = بعدی_فرزند) {
		بعدی_فرزند = فرزند->بعدی;

		بدل (فرزند->type) {
		حالة BOX_جدول_ردیف:
			/* ok */
			گروه_ردیف_شمار++;
			إذا (box_normalise_جدول_ردیف(فرزند, ریشه, ستون_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلوک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_ردیف_گروه:
		حالة BOX_جدول_CELL:
			/* insert implied table row */
			أكد(ردیف_گروه->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ردیف_گروه->style);
			إذا (style == عدم)
				أرجع false;

			ردیف = box_create(عدم, style, true, ردیف_گروه->href,
					ردیف_گروه->هدف, عدم, عدم, c->bctx);
			إذا (ردیف == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			ردیف->type = BOX_جدول_ردیف;

			إذا (فرزند->prev == عدم)
				ردیف_گروه->children = ردیف;
			وإلا
				فرزند->prev->بعدی = ردیف;

			ردیف->prev = فرزند->prev;

			طالما (فرزند != عدم && (
					فرزند->type == BOX_FLEX ||
					فرزند->type == BOX_بلوک ||
					فرزند->type == BOX_INLINE_CONTAINER ||
					فرزند->type == BOX_جدول ||
					فرزند->type == BOX_جدول_ردیف_گروه ||
					فرزند->type == BOX_جدول_CELL)) {
				box_جمع_کن_فرزند(ردیف, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			أكد(ردیف->آخرین != عدم);

			ردیف->آخرین->بعدی = عدم;
			ردیف->بعدی = بعدی_فرزند = فرزند;
			إذا (ردیف->بعدی != عدم)
				ردیف->بعدی->prev = ردیف;
			وإلا
				ردیف_گروه->آخرین = ردیف;
			ردیف->والد = ردیف_گروه;

			گروه_ردیف_شمار++;
			إذا (box_normalise_جدول_ردیف(ردیف, ریشه, ستون_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلوک:
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
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

	إذا (ردیف_گروه->children == عدم) {
#اگر_تعریف_شده BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#پایان_اگر

		أكد(ردیف_گروه->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
		ctx.پایه_url = c->پایه_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ردیف_گروه->style);
		إذا (style == عدم) {
			أرجع false;
		}

		ردیف = box_create(عدم, style, true, ردیف_گروه->href,
				ردیف_گروه->هدف, عدم, عدم, c->bctx);
		إذا (ردیف == عدم) {
			css_computed_style_destroy(style);
			أرجع false;
		}
		ردیف->type = BOX_جدول_ردیف;

		ردیف->والد = ردیف_گروه;
		ردیف_گروه->children = ردیف_گروه->آخرین = ردیف;

		گروه_ردیف_شمار = 1;

		/* Keep table's row count in sync */
		ستون_info->رقم_ردیفها++;
	}

	ردیف_گروه->ردیفها = گروه_ردیف_شمار;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", ردیف_گروه);
#پایان_اگر

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
			  ثابت بنية box *ریشه,
			  بنية بازه_info *spans,
			  html_content *c)
{
	بنية box *جدول_ردیف_گروه;
	بنية box *جدول_ردیف;
	بنية box *جدول_cell;
	بیعلامت صحيح ردیفها_چپ = جدول->ردیفها;
	بیعلامت صحيح گروه_ردیفها_چپ;
	بیعلامت صحيح ستون;
	nscss_select_ctx ctx;

	ctx.ریشه_style = ریشه->style;

	/* Clear span data */
	memset(spans, 0, جدول->columns * حجم(بنية بازه_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	لكل (جدول_ردیف_گروه = جدول->children;
	     جدول_ردیف_گروه != عدم;
	     جدول_ردیف_گروه = جدول_ردیف_گروه->بعدی) {

		گروه_ردیفها_چپ = جدول_ردیف_گروه->ردیفها;

		لكل (جدول_ردیف = جدول_ردیف_گروه->children;
		     جدول_ردیف != عدم;
		     جدول_ردیف = جدول_ردیف->بعدی) {

			لكل (جدول_cell = جدول_ردیف->children;
			     جدول_cell != عدم;
			     جدول_cell = جدول_cell->بعدی) {

				/* colspan = 0 -> colspan = 1 */
				إذا (جدول_cell->columns == 0) {
					جدول_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				إذا (جدول_cell->ردیفها == 0) {
					جدول_cell->ردیفها = گروه_ردیفها_چپ;
				}

				/* limit rowspans within group */
				إذا (جدول_cell->ردیفها > گروه_ردیفها_چپ) {
					جدول_cell->ردیفها = گروه_ردیفها_چپ;
				}

				/* Record span information */
				لكل (ستون = جدول_cell->شروع_ستونی;
						ستون < جدول_cell->شروع_ستونی +
						جدول_cell->columns; ستون++) {
					spans[ستون].ردیف_بازه = جدول_cell->ردیفها;
				}
			}

			/* Reduce span count of each column */
			لكل (ستون = 0; ستون < جدول->columns; ستون++) {
				إذا (spans[ستون].ردیف_بازه == 0) {
					بیعلامت صحيح شروع = ستون;
					css_computed_style *style;
					بنية box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(جدول_ردیف->style != عدم);

					/* Find width of gap */
					طالما (ستون < جدول->columns &&
							spans[ستون].ردیف_بازه ==
							0) {
						ستون++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_نما_پر);
					ctx.پایه_url = c->پایه_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							جدول_ردیف->style);
					إذا (style == عدم)
						أرجع false;

					cell = box_create(عدم, style, true,
							جدول_ردیف->href,
							جدول_ردیف->هدف,
							عدم, عدم, c->bctx);
					إذا (cell == عدم) {
						css_computed_style_destroy(
								style);
						أرجع false;
					}
					cell->type = BOX_جدول_CELL;

					cell->ردیفها = 1;
					cell->columns = ستون - شروع;
					cell->شروع_ستونی = شروع;

					/* Find place to insert cell */
					لكل (prev = جدول_ردیف->children;
							prev != عدم;
							prev = prev->بعدی) {
						إذا (prev->شروع_ستونی +
							prev->columns ==
								شروع)
							اكسر;
						إذا (prev->بعدی == عدم)
							اكسر;
					}

					/* Insert it */
					إذا (prev == عدم) {
						إذا (جدول_ردیف->children != عدم)
							جدول_ردیف->children->
								prev = cell;
						وإلا
							جدول_ردیف->آخرین = cell;

						cell->بعدی =
							جدول_ردیف->children;
						جدول_ردیف->children = cell;
					} وإلا {
						إذا (prev->بعدی != عدم)
							prev->بعدی->prev = cell;
						وإلا
							جدول_ردیف->آخرین = cell;

						cell->بعدی = prev->بعدی;
						prev->بعدی = cell;
						cell->prev = prev;
					}
					cell->والد = جدول_ردیف;
				} وإلا {
					spans[ستون].ردیف_بازه--;
				}
			}

			أكد(ردیفها_چپ > 0);

			ردیفها_چپ--;
		}

		گروه_ردیفها_چپ--;
	}

	أرجع true;
}


ساكن bool
box_normalise_جدول(بنية box *جدول, ثابت بنية box *ریشه, html_content * c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;
	بنية box *ردیف_گروه;
	css_computed_style *style;
	بنية columns ستون_info;
	nscss_select_ctx ctx;

	أكد(جدول != عدم);
	أكد(جدول->type == BOX_جدول);

	ctx.ریشه_style = ریشه->style;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", جدول);
#پایان_اگر

	ستون_info.رقم_columns = 1;
	ستون_info.current_ستونی = 0;
	ستون_info.spans = حافظه_بده(2 * حجم *ستون_info.spans);
	إذا (ستون_info.spans == عدم)
		أرجع false;

	ستون_info.spans[0].ردیف_بازه = ستون_info.spans[1].ردیف_بازه = 0;
	ستون_info.spans[0].auto_ردیف = false;
	ستون_info.spans[1].auto_ردیف = false;
	ستون_info.رقم_ردیفها = 0;

	لكل (فرزند = جدول->children; فرزند != عدم; فرزند = بعدی_فرزند) {
		بعدی_فرزند = فرزند->بعدی;
		بدل (فرزند->type) {
		حالة BOX_جدول_ردیف_گروه:
			/* ok */
			إذا (box_normalise_جدول_ردیف_گروه(فرزند, ریشه,
					&ستون_info, c) == false) {
				حرر(ستون_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلوک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_ردیف:
		حالة BOX_جدول_CELL:
			/* insert implied table row group */
			أكد(جدول->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					جدول->style);
			إذا (style == عدم) {
				حرر(ستون_info.spans);
				أرجع false;
			}

			ردیف_گروه = box_create(عدم, style, true, جدول->href,
					جدول->هدف, عدم, عدم, c->bctx);
			إذا (ردیف_گروه == عدم) {
				css_computed_style_destroy(style);
				حرر(ستون_info.spans);
				أرجع false;
			}

			ردیف_گروه->type = BOX_جدول_ردیف_گروه;

			إذا (فرزند->prev == عدم)
				جدول->children = ردیف_گروه;
			وإلا
				فرزند->prev->بعدی = ردیف_گروه;

			ردیف_گروه->prev = فرزند->prev;

			طالما (فرزند != عدم && (
					فرزند->type == BOX_FLEX ||
					فرزند->type == BOX_بلوک ||
					فرزند->type == BOX_INLINE_CONTAINER ||
					فرزند->type == BOX_جدول ||
					فرزند->type == BOX_جدول_ردیف ||
					فرزند->type == BOX_جدول_CELL)) {
				box_جمع_کن_فرزند(ردیف_گروه, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			أكد(ردیف_گروه->آخرین != عدم);

			ردیف_گروه->آخرین->بعدی = عدم;
			ردیف_گروه->بعدی = بعدی_فرزند = فرزند;
			إذا (ردیف_گروه->بعدی != عدم)
				ردیف_گروه->بعدی->prev = ردیف_گروه;
			وإلا
				جدول->آخرین = ردیف_گروه;
			ردیف_گروه->والد = جدول;

			إذا (box_normalise_جدول_ردیف_گروه(ردیف_گروه, ریشه,
					&ستون_info, c) == false) {
				حرر(ستون_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلوک:
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			fprintf(stderr, "%i\n", فرزند->type);
			أكد(0);
		}
	}

	جدول->columns = ستون_info.رقم_columns;
	جدول->ردیفها = ستون_info.رقم_ردیفها;

	إذا (جدول->children == عدم) {
		بنية box *ردیف;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#پایان_اگر

		أكد(جدول->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
		ctx.پایه_url = c->پایه_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				جدول->style);
		إذا (style == عدم) {
			حرر(ستون_info.spans);
			أرجع false;
		}

		ردیف_گروه = box_create(عدم, style, true, جدول->href,
				جدول->هدف, عدم, عدم, c->bctx);
		إذا (ردیف_گروه == عدم) {
			css_computed_style_destroy(style);
			حرر(ستون_info.spans);
			أرجع false;
		}
		ردیف_گروه->type = BOX_جدول_ردیف_گروه;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ردیف_گروه->style);
		إذا (style == عدم) {
			box_free(ردیف_گروه);
			حرر(ستون_info.spans);
			أرجع false;
		}

		ردیف = box_create(عدم, style, true, ردیف_گروه->href,
				ردیف_گروه->هدف, عدم, عدم, c->bctx);
		إذا (ردیف == عدم) {
			css_computed_style_destroy(style);
			box_free(ردیف_گروه);
			حرر(ستون_info.spans);
			أرجع false;
		}
		ردیف->type = BOX_جدول_ردیف;

		ردیف->والد = ردیف_گروه;
		ردیف_گروه->children = ردیف_گروه->آخرین = ردیف;

		ردیف_گروه->والد = جدول;
		جدول->children = جدول->آخرین = ردیف_گروه;

		جدول->ردیفها = 1;
	}

	إذا (box_normalise_جدول_spans(جدول, ریشه, ستون_info.spans, c) == false) {
		حرر(ستون_info.spans);
		أرجع false;
	}

	حرر(ستون_info.spans);

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", جدول);
#پایان_اگر

	أرجع true;
}

ساكن bool box_normalise_flex(
		بنية box *flex_container,
		ثابت بنية box *ریشه,
		html_content *c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;
	بنية box *implied_flex_قلم;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != عدم);
	أكد(ریشه != عدم);

	ctx.ریشه_style = ریشه->style;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#پایان_اگر

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	لكل (فرزند = flex_container->children; فرزند != عدم; فرزند = بعدی_فرزند) {
#اگر_تعریف_شده BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				فرزند, فرزند->type);
#پایان_اگر

		بعدی_فرزند = فرزند->بعدی;	/* child may be destroyed */

		بدل (فرزند->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلوک:
			/* ok */
			إذا (box_normalise_بلوک(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_قلم = box_create(عدم, style, true,
					flex_container->href,
					flex_container->هدف,
					عدم, عدم, c->bctx);
			إذا (implied_flex_قلم == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_قلم->type = BOX_بلوک;

			إذا (فرزند->prev == عدم)
				flex_container->children = implied_flex_قلم;
			وإلا
				فرزند->prev->بعدی = implied_flex_قلم;

			implied_flex_قلم->prev = فرزند->prev;

			طالما (فرزند != عدم &&
					فرزند->type == BOX_INLINE_CONTAINER) {
				box_جمع_کن_فرزند(implied_flex_قلم, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			implied_flex_قلم->آخرین->بعدی = عدم;
			implied_flex_قلم->بعدی = بعدی_فرزند = فرزند;
			إذا (implied_flex_قلم->بعدی != عدم)
				implied_flex_قلم->بعدی->prev = implied_flex_قلم;
			وإلا
				flex_container->آخرین = implied_flex_قلم;
			implied_flex_قلم->والد = flex_container;

			إذا (box_normalise_بلوک(implied_flex_قلم,
					ریشه, c) == false)
				أرجع false;
			اكسر;

		حالة BOX_جدول:
			إذا (box_normalise_جدول(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلوک:
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_ردیف_گروه:
		حالة BOX_جدول_ردیف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_قلم = box_create(عدم, style, true,
					flex_container->href,
					flex_container->هدف,
					عدم, عدم, c->bctx);
			إذا (implied_flex_قلم == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_قلم->type = BOX_جدول;

			إذا (فرزند->prev == عدم)
				flex_container->children = implied_flex_قلم;
			وإلا
				فرزند->prev->بعدی = implied_flex_قلم;

			implied_flex_قلم->prev = فرزند->prev;

			طالما (فرزند != عدم && (
					فرزند->type == BOX_جدول_ردیف_گروه ||
					فرزند->type == BOX_جدول_ردیف ||
					فرزند->type == BOX_جدول_CELL)) {
				box_جمع_کن_فرزند(implied_flex_قلم, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			implied_flex_قلم->آخرین->بعدی = عدم;
			implied_flex_قلم->بعدی = بعدی_فرزند = فرزند;
			إذا (implied_flex_قلم->بعدی != عدم)
				implied_flex_قلم->بعدی->prev = implied_flex_قلم;
			وإلا
				flex_container->آخرین = implied_flex_قلم;
			implied_flex_قلم->والد = flex_container;

			إذا (box_normalise_جدول(implied_flex_قلم,
					ریشه, c) == false)
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
			       ثابت بنية box *ریشه,
			       html_content * c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;

	أكد(cont != عدم);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#پایان_اگر

	لكل (فرزند = cont->children; فرزند != عدم; فرزند = بعدی_فرزند) {
		بعدی_فرزند = فرزند->بعدی;
		بدل (فرزند->type) {
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_BR:
		حالة BOX_متن:
			/* ok */
			اكسر;
		حالة BOX_INLINE_بلوک:
			/* ok */
			إذا (box_normalise_بلوک(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_FLEX:
			/* ok */
			إذا (box_normalise_flex(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
			/* ok */
			أكد(فرزند->children != عدم);

			بدل (فرزند->children->type) {
			حالة BOX_بلوک:
				إذا (box_normalise_بلوک(فرزند->children, ریشه,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_جدول:
				إذا (box_normalise_جدول(فرزند->children, ریشه,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_FLEX:
				إذا (box_normalise_flex(فرزند->children, ریشه,
						c) == false)
					أرجع false;
				اكسر;
			افتراضي:
				أكد(0);
			}

			إذا (فرزند->children == عدم) {
				/* the child has destroyed itself: remove float */
				إذا (فرزند->prev == عدم)
					فرزند->والد->children = فرزند->بعدی;
				وإلا
					فرزند->prev->بعدی = فرزند->بعدی;
				إذا (فرزند->بعدی != عدم)
					فرزند->بعدی->prev = فرزند->prev;
				وإلا
					فرزند->والد->آخرین = فرزند->prev;

				box_free(فرزند);
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلوک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_ردیف_گروه:
		حالة BOX_جدول_ردیف:
		حالة BOX_جدول_CELL:
		افتراضي:
			أكد(0);
		}
	}

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#پایان_اگر

	أرجع true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_بلوک(بنية box *بلوک, ثابت بنية box *ریشه, html_content *c)
{
	بنية box *فرزند;
	بنية box *بعدی_فرزند;
	بنية box *جدول;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(بلوک != عدم);
	أكد(ریشه != عدم);

	ctx.ریشه_style = ریشه->style;

#اگر_تعریف_شده BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", بلوک, بلوک->type);
#پایان_اگر

	أكد(بلوک->type == BOX_بلوک || بلوک->type == BOX_INLINE_بلوک ||
			بلوک->type == BOX_جدول_CELL);

	لكل (فرزند = بلوک->children; فرزند != عدم; فرزند = بعدی_فرزند) {
#اگر_تعریف_شده BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", فرزند,
		      فرزند->type);
#پایان_اگر

		بعدی_فرزند = فرزند->بعدی;	/* child may be destroyed */

		بدل (فرزند->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلوک:
			/* ok */
			إذا (box_normalise_بلوک(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			إذا (box_normalise_inline_container(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_جدول:
			إذا (box_normalise_جدول(فرزند, ریشه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_انتها:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلوک:
		حالة BOX_FLOAT_چپ:
		حالة BOX_FLOAT_راست:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_ردیف_گروه:
		حالة BOX_جدول_ردیف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(بلوک->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_نما_پر);
			ctx.پایه_url = c->پایه_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					بلوک->style);
			إذا (style == عدم)
				أرجع false;

			جدول = box_create(عدم, style, true, بلوک->href,
					بلوک->هدف, عدم, عدم, c->bctx);
			إذا (جدول == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			جدول->type = BOX_جدول;

			إذا (فرزند->prev == عدم)
				بلوک->children = جدول;
			وإلا
				فرزند->prev->بعدی = جدول;

			جدول->prev = فرزند->prev;

			طالما (فرزند != عدم && (
					فرزند->type == BOX_جدول_ردیف_گروه ||
					فرزند->type == BOX_جدول_ردیف ||
					فرزند->type == BOX_جدول_CELL)) {
				box_جمع_کن_فرزند(جدول, فرزند);

				بعدی_فرزند = فرزند->بعدی;
				فرزند->بعدی = عدم;
				فرزند = بعدی_فرزند;
			}

			جدول->آخرین->بعدی = عدم;
			جدول->بعدی = بعدی_فرزند = فرزند;
			إذا (جدول->بعدی != عدم)
				جدول->بعدی->prev = جدول;
			وإلا
				بلوک->آخرین = جدول;
			جدول->والد = بلوک;

			إذا (box_normalise_جدول(جدول, ریشه, c) == false)
				أرجع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	أرجع true;
}
