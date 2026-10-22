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
#وصف_رد BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
بنية وسعت_info {
	/** Number of rows this cell spans */
	بنانشان صحيح صف_وسعت;
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
	بنانشان صحيح current_ڪالم;
	/** Number of columns in main part of table 1..max columns */
	بنانشان صحيح انگ_columns;
	/** Information about columns in main table, array [0, num_columns) */
	بنية وسعت_info *spans;
	/** Number of rows in table */
	بنانشان صحيح انگ_صفون;
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
calculate_جدول_صف(بنية columns *کانو_info,
		    بنانشان صحيح کانو_وسعت,
		    بنانشان صحيح صف_وسعت,
		    بنانشان صحيح *شروعات_ڪالم,
		    بنية box *cell)
{
	بنانشان صحيح cell_شروعات_کانو = کانو_info->current_ڪالم;
	بنانشان صحيح cell_پڇاڙي_کانو;
	بنانشان صحيح i;
	بنية وسعت_info *spans;
	بنية box *rg = cell->والد->والد; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	طالما (کانو_info->spans[cell_شروعات_کانو].صف_وسعت != 0 &&
			کانو_info->spans[cell_شروعات_کانو].rg == rg) {
		cell_شروعات_کانو++;
	}

	/* Update current column with calculated start */
	کانو_info->current_ڪالم = cell_شروعات_کانو;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	إذا (کانو_وسعت == 0)
		کانو_وسعت = 1;

	cell_پڇاڙي_کانو = cell_شروعات_کانو + کانو_وسعت;

	إذا (کانو_info->انگ_columns < cell_پڇاڙي_کانو) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(کانو_info->spans,
				حجم *spans * (cell_پڇاڙي_کانو + 1));
		إذا (spans == عدم)
			أرجع false;

		کانو_info->spans = spans;
		کانو_info->انگ_columns = cell_پڇاڙي_کانو;

		/* Mark new final column as sentinel */
		کانو_info->spans[cell_پڇاڙي_کانو].صف_وسعت = 0;
		کانو_info->spans[cell_پڇاڙي_کانو].auto_صف = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	لكل (i = cell_شروعات_کانو; i < cell_پڇاڙي_کانو; i++) {
		کانو_info->spans[i].صف_وسعت = (صف_وسعت == 0) ? 1 : صف_وسعت;
		کانو_info->spans[i].auto_صف = (صف_وسعت == 0);
		کانو_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	کانو_info->current_ڪالم = cell_پڇاڙي_کانو;

	*شروعات_ڪالم = cell_شروعات_کانو;

	أرجع true;
}


ساكن bool
box_normalise_جدول_صف(بنية box *صف,
			ثابت بنية box *پاڙ,
			بنية columns *کانو_info,
			html_content * c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;
	بنية box *cell = عدم;
	css_computed_style *style;
	بنانشان صحيح i;
	nscss_select_ctx ctx;

	أكد(صف != عدم);
	أكد(صف->type == BOX_جدول_صف);

	ctx.پاڙ_style = پاڙ->style;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", صف);
#جيڪڏهن_پورو

	لكل (ٻار = صف->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
		ايندڙ_ٻار = ٻار->ايندڙ;

		بدل (ٻار->type) {
		حالة BOX_جدول_CELL:
			/* ok */
			إذا (box_normalise_بلاڪ(ٻار, پاڙ, c) == false)
				أرجع false;
			cell = ٻار;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاڪ:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروهه:
		حالة BOX_جدول_صف:
			/* insert implied table cell */
			أكد(صف->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف->style);
			إذا (style == عدم)
				أرجع false;

			cell = box_create(عدم, style, true, صف->href,
					صف->هدف, عدم, عدم, c->bctx);
			إذا (cell == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			cell->type = BOX_جدول_CELL;

			إذا (ٻار->prev == عدم)
				صف->children = cell;
			وإلا
				ٻار->prev->ايندڙ = cell;

			cell->prev = ٻار->prev;

			طالما (ٻار != عدم && (
					ٻار->type == BOX_FLEX ||
					ٻار->type == BOX_بلاڪ ||
					ٻار->type == BOX_INLINE_CONTAINER ||
					ٻار->type == BOX_جدول ||
					ٻار->type == BOX_جدول_صف_گروهه ||
					ٻار->type == BOX_جدول_صف)) {
				box_جوڙيو_ٻار(cell, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			أكد(cell->آخري != عدم);

			cell->آخري->ايندڙ = عدم;
			cell->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (cell->ايندڙ != عدم)
				cell->ايندڙ->prev = cell;
			وإلا
				صف->آخري = cell;
			cell->والد = صف;

			إذا (box_normalise_بلاڪ(cell, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاڪ:
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}

		إذا (calculate_جدول_صف(کانو_info, cell->columns, cell->صفون,
				&cell->شروعات_ڪالم, cell) == false)
			أرجع false;
	}


	/* Update row spanning details for all columns */
	لكل (i = 0; i < کانو_info->انگ_columns; i++) {
		إذا (کانو_info->spans[i].صف_وسعت != 0 &&
				کانو_info->spans[i].auto_صف == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			کانو_info->spans[i].صف_وسعت--;
		}
	}

	/* Reset current column for next row */
	کانو_info->current_ڪالم = 0;

	/* Increment row counter */
	کانو_info->انگ_صفون++;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", صف);
#جيڪڏهن_پورو

	أرجع true;
}


ساكن bool
box_normalise_جدول_صف_گروهه(بنية box *صف_گروهه,
			      ثابت بنية box *پاڙ,
			      بنية columns *کانو_info,
			      html_content * c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;
	بنية box *صف;
	css_computed_style *style;
	nscss_select_ctx ctx;
	بنانشان صحيح گروهه_صف_ڳڻپ = 0;

	أكد(صف_گروهه != 0);
	أكد(صف_گروهه->type == BOX_جدول_صف_گروهه);

	ctx.پاڙ_style = پاڙ->style;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", صف_گروهه);
#جيڪڏهن_پورو

	لكل (ٻار = صف_گروهه->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
		ايندڙ_ٻار = ٻار->ايندڙ;

		بدل (ٻار->type) {
		حالة BOX_جدول_صف:
			/* ok */
			گروهه_صف_ڳڻپ++;
			إذا (box_normalise_جدول_صف(ٻار, پاڙ, کانو_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاڪ:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروهه:
		حالة BOX_جدول_CELL:
			/* insert implied table row */
			أكد(صف_گروهه->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					صف_گروهه->style);
			إذا (style == عدم)
				أرجع false;

			صف = box_create(عدم, style, true, صف_گروهه->href,
					صف_گروهه->هدف, عدم, عدم, c->bctx);
			إذا (صف == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			صف->type = BOX_جدول_صف;

			إذا (ٻار->prev == عدم)
				صف_گروهه->children = صف;
			وإلا
				ٻار->prev->ايندڙ = صف;

			صف->prev = ٻار->prev;

			طالما (ٻار != عدم && (
					ٻار->type == BOX_FLEX ||
					ٻار->type == BOX_بلاڪ ||
					ٻار->type == BOX_INLINE_CONTAINER ||
					ٻار->type == BOX_جدول ||
					ٻار->type == BOX_جدول_صف_گروهه ||
					ٻار->type == BOX_جدول_CELL)) {
				box_جوڙيو_ٻار(صف, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			أكد(صف->آخري != عدم);

			صف->آخري->ايندڙ = عدم;
			صف->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (صف->ايندڙ != عدم)
				صف->ايندڙ->prev = صف;
			وإلا
				صف_گروهه->آخري = صف;
			صف->والد = صف_گروهه;

			گروهه_صف_ڳڻپ++;
			إذا (box_normalise_جدول_صف(صف, پاڙ, کانو_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاڪ:
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
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

	إذا (صف_گروهه->children == عدم) {
#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#جيڪڏهن_پورو

		أكد(صف_گروهه->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
		ctx.بنياد_url = c->بنياد_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروهه->style);
		إذا (style == عدم) {
			أرجع false;
		}

		صف = box_create(عدم, style, true, صف_گروهه->href,
				صف_گروهه->هدف, عدم, عدم, c->bctx);
		إذا (صف == عدم) {
			css_computed_style_destroy(style);
			أرجع false;
		}
		صف->type = BOX_جدول_صف;

		صف->والد = صف_گروهه;
		صف_گروهه->children = صف_گروهه->آخري = صف;

		گروهه_صف_ڳڻپ = 1;

		/* Keep table's row count in sync */
		کانو_info->انگ_صفون++;
	}

	صف_گروهه->صفون = گروهه_صف_ڳڻپ;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", صف_گروهه);
#جيڪڏهن_پورو

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
			  ثابت بنية box *پاڙ,
			  بنية وسعت_info *spans,
			  html_content *c)
{
	بنية box *جدول_صف_گروهه;
	بنية box *جدول_صف;
	بنية box *جدول_cell;
	بنانشان صحيح صفون_کاٻو = جدول->صفون;
	بنانشان صحيح گروهه_صفون_کاٻو;
	بنانشان صحيح کانو;
	nscss_select_ctx ctx;

	ctx.پاڙ_style = پاڙ->style;

	/* Clear span data */
	memset(spans, 0, جدول->columns * حجم(بنية وسعت_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	لكل (جدول_صف_گروهه = جدول->children;
	     جدول_صف_گروهه != عدم;
	     جدول_صف_گروهه = جدول_صف_گروهه->ايندڙ) {

		گروهه_صفون_کاٻو = جدول_صف_گروهه->صفون;

		لكل (جدول_صف = جدول_صف_گروهه->children;
		     جدول_صف != عدم;
		     جدول_صف = جدول_صف->ايندڙ) {

			لكل (جدول_cell = جدول_صف->children;
			     جدول_cell != عدم;
			     جدول_cell = جدول_cell->ايندڙ) {

				/* colspan = 0 -> colspan = 1 */
				إذا (جدول_cell->columns == 0) {
					جدول_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				إذا (جدول_cell->صفون == 0) {
					جدول_cell->صفون = گروهه_صفون_کاٻو;
				}

				/* limit rowspans within group */
				إذا (جدول_cell->صفون > گروهه_صفون_کاٻو) {
					جدول_cell->صفون = گروهه_صفون_کاٻو;
				}

				/* Record span information */
				لكل (کانو = جدول_cell->شروعات_ڪالم;
						کانو < جدول_cell->شروعات_ڪالم +
						جدول_cell->columns; کانو++) {
					spans[کانو].صف_وسعت = جدول_cell->صفون;
				}
			}

			/* Reduce span count of each column */
			لكل (کانو = 0; کانو < جدول->columns; کانو++) {
				إذا (spans[کانو].صف_وسعت == 0) {
					بنانشان صحيح شروعات = کانو;
					css_computed_style *style;
					بنية box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(جدول_صف->style != عدم);

					/* Find width of gap */
					طالما (کانو < جدول->columns &&
							spans[کانو].صف_وسعت ==
							0) {
						کانو++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
					ctx.بنياد_url = c->بنياد_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							جدول_صف->style);
					إذا (style == عدم)
						أرجع false;

					cell = box_create(عدم, style, true,
							جدول_صف->href,
							جدول_صف->هدف,
							عدم, عدم, c->bctx);
					إذا (cell == عدم) {
						css_computed_style_destroy(
								style);
						أرجع false;
					}
					cell->type = BOX_جدول_CELL;

					cell->صفون = 1;
					cell->columns = کانو - شروعات;
					cell->شروعات_ڪالم = شروعات;

					/* Find place to insert cell */
					لكل (prev = جدول_صف->children;
							prev != عدم;
							prev = prev->ايندڙ) {
						إذا (prev->شروعات_ڪالم +
							prev->columns ==
								شروعات)
							اكسر;
						إذا (prev->ايندڙ == عدم)
							اكسر;
					}

					/* Insert it */
					إذا (prev == عدم) {
						إذا (جدول_صف->children != عدم)
							جدول_صف->children->
								prev = cell;
						وإلا
							جدول_صف->آخري = cell;

						cell->ايندڙ =
							جدول_صف->children;
						جدول_صف->children = cell;
					} وإلا {
						إذا (prev->ايندڙ != عدم)
							prev->ايندڙ->prev = cell;
						وإلا
							جدول_صف->آخري = cell;

						cell->ايندڙ = prev->ايندڙ;
						prev->ايندڙ = cell;
						cell->prev = prev;
					}
					cell->والد = جدول_صف;
				} وإلا {
					spans[کانو].صف_وسعت--;
				}
			}

			أكد(صفون_کاٻو > 0);

			صفون_کاٻو--;
		}

		گروهه_صفون_کاٻو--;
	}

	أرجع true;
}


ساكن bool
box_normalise_جدول(بنية box *جدول, ثابت بنية box *پاڙ, html_content * c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;
	بنية box *صف_گروهه;
	css_computed_style *style;
	بنية columns کانو_info;
	nscss_select_ctx ctx;

	أكد(جدول != عدم);
	أكد(جدول->type == BOX_جدول);

	ctx.پاڙ_style = پاڙ->style;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", جدول);
#جيڪڏهن_پورو

	کانو_info.انگ_columns = 1;
	کانو_info.current_ڪالم = 0;
	کانو_info.spans = يادگيري_ڏيو(2 * حجم *کانو_info.spans);
	إذا (کانو_info.spans == عدم)
		أرجع false;

	کانو_info.spans[0].صف_وسعت = کانو_info.spans[1].صف_وسعت = 0;
	کانو_info.spans[0].auto_صف = false;
	کانو_info.spans[1].auto_صف = false;
	کانو_info.انگ_صفون = 0;

	لكل (ٻار = جدول->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
		ايندڙ_ٻار = ٻار->ايندڙ;
		بدل (ٻار->type) {
		حالة BOX_جدول_صف_گروهه:
			/* ok */
			إذا (box_normalise_جدول_صف_گروهه(ٻار, پاڙ,
					&کانو_info, c) == false) {
				حرر(کانو_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاڪ:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table row group */
			أكد(جدول->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					جدول->style);
			إذا (style == عدم) {
				حرر(کانو_info.spans);
				أرجع false;
			}

			صف_گروهه = box_create(عدم, style, true, جدول->href,
					جدول->هدف, عدم, عدم, c->bctx);
			إذا (صف_گروهه == عدم) {
				css_computed_style_destroy(style);
				حرر(کانو_info.spans);
				أرجع false;
			}

			صف_گروهه->type = BOX_جدول_صف_گروهه;

			إذا (ٻار->prev == عدم)
				جدول->children = صف_گروهه;
			وإلا
				ٻار->prev->ايندڙ = صف_گروهه;

			صف_گروهه->prev = ٻار->prev;

			طالما (ٻار != عدم && (
					ٻار->type == BOX_FLEX ||
					ٻار->type == BOX_بلاڪ ||
					ٻار->type == BOX_INLINE_CONTAINER ||
					ٻار->type == BOX_جدول ||
					ٻار->type == BOX_جدول_صف ||
					ٻار->type == BOX_جدول_CELL)) {
				box_جوڙيو_ٻار(صف_گروهه, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			أكد(صف_گروهه->آخري != عدم);

			صف_گروهه->آخري->ايندڙ = عدم;
			صف_گروهه->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (صف_گروهه->ايندڙ != عدم)
				صف_گروهه->ايندڙ->prev = صف_گروهه;
			وإلا
				جدول->آخري = صف_گروهه;
			صف_گروهه->والد = جدول;

			إذا (box_normalise_جدول_صف_گروهه(صف_گروهه, پاڙ,
					&کانو_info, c) == false) {
				حرر(کانو_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاڪ:
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			fprintf(stderr, "%i\n", ٻار->type);
			أكد(0);
		}
	}

	جدول->columns = کانو_info.انگ_columns;
	جدول->صفون = کانو_info.انگ_صفون;

	إذا (جدول->children == عدم) {
		بنية box *صف;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#جيڪڏهن_پورو

		أكد(جدول->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
		ctx.بنياد_url = c->بنياد_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				جدول->style);
		إذا (style == عدم) {
			حرر(کانو_info.spans);
			أرجع false;
		}

		صف_گروهه = box_create(عدم, style, true, جدول->href,
				جدول->هدف, عدم, عدم, c->bctx);
		إذا (صف_گروهه == عدم) {
			css_computed_style_destroy(style);
			حرر(کانو_info.spans);
			أرجع false;
		}
		صف_گروهه->type = BOX_جدول_صف_گروهه;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				صف_گروهه->style);
		إذا (style == عدم) {
			box_free(صف_گروهه);
			حرر(کانو_info.spans);
			أرجع false;
		}

		صف = box_create(عدم, style, true, صف_گروهه->href,
				صف_گروهه->هدف, عدم, عدم, c->bctx);
		إذا (صف == عدم) {
			css_computed_style_destroy(style);
			box_free(صف_گروهه);
			حرر(کانو_info.spans);
			أرجع false;
		}
		صف->type = BOX_جدول_صف;

		صف->والد = صف_گروهه;
		صف_گروهه->children = صف_گروهه->آخري = صف;

		صف_گروهه->والد = جدول;
		جدول->children = جدول->آخري = صف_گروهه;

		جدول->صفون = 1;
	}

	إذا (box_normalise_جدول_spans(جدول, پاڙ, کانو_info.spans, c) == false) {
		حرر(کانو_info.spans);
		أرجع false;
	}

	حرر(کانو_info.spans);

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", جدول);
#جيڪڏهن_پورو

	أرجع true;
}

ساكن bool box_normalise_flex(
		بنية box *flex_container,
		ثابت بنية box *پاڙ,
		html_content *c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;
	بنية box *implied_flex_شي;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != عدم);
	أكد(پاڙ != عدم);

	ctx.پاڙ_style = پاڙ->style;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#جيڪڏهن_پورو

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	لكل (ٻار = flex_container->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				ٻار, ٻار->type);
#جيڪڏهن_پورو

		ايندڙ_ٻار = ٻار->ايندڙ;	/* child may be destroyed */

		بدل (ٻار->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاڪ:
			/* ok */
			إذا (box_normalise_بلاڪ(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_شي = box_create(عدم, style, true,
					flex_container->href,
					flex_container->هدف,
					عدم, عدم, c->bctx);
			إذا (implied_flex_شي == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_شي->type = BOX_بلاڪ;

			إذا (ٻار->prev == عدم)
				flex_container->children = implied_flex_شي;
			وإلا
				ٻار->prev->ايندڙ = implied_flex_شي;

			implied_flex_شي->prev = ٻار->prev;

			طالما (ٻار != عدم &&
					ٻار->type == BOX_INLINE_CONTAINER) {
				box_جوڙيو_ٻار(implied_flex_شي, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			implied_flex_شي->آخري->ايندڙ = عدم;
			implied_flex_شي->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (implied_flex_شي->ايندڙ != عدم)
				implied_flex_شي->ايندڙ->prev = implied_flex_شي;
			وإلا
				flex_container->آخري = implied_flex_شي;
			implied_flex_شي->والد = flex_container;

			إذا (box_normalise_بلاڪ(implied_flex_شي,
					پاڙ, c) == false)
				أرجع false;
			اكسر;

		حالة BOX_جدول:
			إذا (box_normalise_جدول(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاڪ:
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_صف_گروهه:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_شي = box_create(عدم, style, true,
					flex_container->href,
					flex_container->هدف,
					عدم, عدم, c->bctx);
			إذا (implied_flex_شي == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_شي->type = BOX_جدول;

			إذا (ٻار->prev == عدم)
				flex_container->children = implied_flex_شي;
			وإلا
				ٻار->prev->ايندڙ = implied_flex_شي;

			implied_flex_شي->prev = ٻار->prev;

			طالما (ٻار != عدم && (
					ٻار->type == BOX_جدول_صف_گروهه ||
					ٻار->type == BOX_جدول_صف ||
					ٻار->type == BOX_جدول_CELL)) {
				box_جوڙيو_ٻار(implied_flex_شي, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			implied_flex_شي->آخري->ايندڙ = عدم;
			implied_flex_شي->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (implied_flex_شي->ايندڙ != عدم)
				implied_flex_شي->ايندڙ->prev = implied_flex_شي;
			وإلا
				flex_container->آخري = implied_flex_شي;
			implied_flex_شي->والد = flex_container;

			إذا (box_normalise_جدول(implied_flex_شي,
					پاڙ, c) == false)
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
			       ثابت بنية box *پاڙ,
			       html_content * c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;

	أكد(cont != عدم);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#جيڪڏهن_پورو

	لكل (ٻار = cont->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
		ايندڙ_ٻار = ٻار->ايندڙ;
		بدل (ٻار->type) {
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_BR:
		حالة BOX_متن:
			/* ok */
			اكسر;
		حالة BOX_INLINE_بلاڪ:
			/* ok */
			إذا (box_normalise_بلاڪ(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_FLEX:
			/* ok */
			إذا (box_normalise_flex(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
			/* ok */
			أكد(ٻار->children != عدم);

			بدل (ٻار->children->type) {
			حالة BOX_بلاڪ:
				إذا (box_normalise_بلاڪ(ٻار->children, پاڙ,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_جدول:
				إذا (box_normalise_جدول(ٻار->children, پاڙ,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_FLEX:
				إذا (box_normalise_flex(ٻار->children, پاڙ,
						c) == false)
					أرجع false;
				اكسر;
			افتراضي:
				أكد(0);
			}

			إذا (ٻار->children == عدم) {
				/* the child has destroyed itself: remove float */
				إذا (ٻار->prev == عدم)
					ٻار->والد->children = ٻار->ايندڙ;
				وإلا
					ٻار->prev->ايندڙ = ٻار->ايندڙ;
				إذا (ٻار->ايندڙ != عدم)
					ٻار->ايندڙ->prev = ٻار->prev;
				وإلا
					ٻار->والد->آخري = ٻار->prev;

				box_free(ٻار);
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاڪ:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_صف_گروهه:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
		افتراضي:
			أكد(0);
		}
	}

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#جيڪڏهن_پورو

	أرجع true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_بلاڪ(بنية box *بلاڪ, ثابت بنية box *پاڙ, html_content *c)
{
	بنية box *ٻار;
	بنية box *ايندڙ_ٻار;
	بنية box *جدول;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(بلاڪ != عدم);
	أكد(پاڙ != عدم);

	ctx.پاڙ_style = پاڙ->style;

#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", بلاڪ, بلاڪ->type);
#جيڪڏهن_پورو

	أكد(بلاڪ->type == BOX_بلاڪ || بلاڪ->type == BOX_INLINE_بلاڪ ||
			بلاڪ->type == BOX_جدول_CELL);

	لكل (ٻار = بلاڪ->children; ٻار != عدم; ٻار = ايندڙ_ٻار) {
#جيڪڏهن_وصف_ٿيل BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", ٻار,
		      ٻار->type);
#جيڪڏهن_پورو

		ايندڙ_ٻار = ٻار->ايندڙ;	/* child may be destroyed */

		بدل (ٻار->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاڪ:
			/* ok */
			إذا (box_normalise_بلاڪ(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			إذا (box_normalise_inline_container(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_جدول:
			إذا (box_normalise_جدول(ٻار, پاڙ, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پڇاڙي:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاڪ:
		حالة BOX_FLOAT_کاٻو:
		حالة BOX_FLOAT_ساڄو:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_صف_گروهه:
		حالة BOX_جدول_صف:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(بلاڪ->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_گهڻائي_ڀريل);
			ctx.بنياد_url = c->بنياد_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					بلاڪ->style);
			إذا (style == عدم)
				أرجع false;

			جدول = box_create(عدم, style, true, بلاڪ->href,
					بلاڪ->هدف, عدم, عدم, c->bctx);
			إذا (جدول == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			جدول->type = BOX_جدول;

			إذا (ٻار->prev == عدم)
				بلاڪ->children = جدول;
			وإلا
				ٻار->prev->ايندڙ = جدول;

			جدول->prev = ٻار->prev;

			طالما (ٻار != عدم && (
					ٻار->type == BOX_جدول_صف_گروهه ||
					ٻار->type == BOX_جدول_صف ||
					ٻار->type == BOX_جدول_CELL)) {
				box_جوڙيو_ٻار(جدول, ٻار);

				ايندڙ_ٻار = ٻار->ايندڙ;
				ٻار->ايندڙ = عدم;
				ٻار = ايندڙ_ٻار;
			}

			جدول->آخري->ايندڙ = عدم;
			جدول->ايندڙ = ايندڙ_ٻار = ٻار;
			إذا (جدول->ايندڙ != عدم)
				جدول->ايندڙ->prev = جدول;
			وإلا
				بلاڪ->آخري = جدول;
			جدول->والد = بلاڪ;

			إذا (box_normalise_جدول(جدول, پاڙ, c) == false)
				أرجع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	أرجع true;
}
