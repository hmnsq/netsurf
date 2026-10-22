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
#تعریف_لغوه BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
بنية پراختیا_info {
	/** Number of rows this cell spans */
	بینښه صحيح لیکه_پراختیا;
	/** Row group of cell */
	بنية box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_لیکه;
};

/**
 * Column record for a table
 */
بنية columns {
	/** Current column index */
	بینښه صحيح current_ستنه;
	/** Number of columns in main part of table 1..max columns */
	بینښه صحيح عدد_columns;
	/** Information about columns in main table, array [0, num_columns) */
	بنية پراختیا_info *spans;
	/** Number of rows in table */
	بینښه صحيح عدد_لیکې;
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
calculate_جدول_لیکه(بنية columns *ستون_info,
		    بینښه صحيح ستون_پراختیا,
		    بینښه صحيح لیکه_پراختیا,
		    بینښه صحيح *پیل_ستنه,
		    بنية box *cell)
{
	بینښه صحيح cell_پیل_ستون = ستون_info->current_ستنه;
	بینښه صحيح cell_پای_ټکی_ستون;
	بینښه صحيح i;
	بنية پراختیا_info *spans;
	بنية box *rg = cell->پلار->پلار; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	طالما (ستون_info->spans[cell_پیل_ستون].لیکه_پراختیا != 0 &&
			ستون_info->spans[cell_پیل_ستون].rg == rg) {
		cell_پیل_ستون++;
	}

	/* Update current column with calculated start */
	ستون_info->current_ستنه = cell_پیل_ستون;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	إذا (ستون_پراختیا == 0)
		ستون_پراختیا = 1;

	cell_پای_ټکی_ستون = cell_پیل_ستون + ستون_پراختیا;

	إذا (ستون_info->عدد_columns < cell_پای_ټکی_ستون) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(ستون_info->spans,
				حجم *spans * (cell_پای_ټکی_ستون + 1));
		إذا (spans == عدم)
			أرجع false;

		ستون_info->spans = spans;
		ستون_info->عدد_columns = cell_پای_ټکی_ستون;

		/* Mark new final column as sentinel */
		ستون_info->spans[cell_پای_ټکی_ستون].لیکه_پراختیا = 0;
		ستون_info->spans[cell_پای_ټکی_ستون].auto_لیکه = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	لكل (i = cell_پیل_ستون; i < cell_پای_ټکی_ستون; i++) {
		ستون_info->spans[i].لیکه_پراختیا = (لیکه_پراختیا == 0) ? 1 : لیکه_پراختیا;
		ستون_info->spans[i].auto_لیکه = (لیکه_پراختیا == 0);
		ستون_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	ستون_info->current_ستنه = cell_پای_ټکی_ستون;

	*پیل_ستنه = cell_پیل_ستون;

	أرجع true;
}


ساكن bool
box_normalise_جدول_لیکه(بنية box *لیکه,
			ثابت بنية box *ریښه,
			بنية columns *ستون_info,
			html_content * c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;
	بنية box *cell = عدم;
	css_computed_style *style;
	بینښه صحيح i;
	nscss_select_ctx ctx;

	أكد(لیکه != عدم);
	أكد(لیکه->type == BOX_جدول_لیکه);

	ctx.ریښه_style = ریښه->style;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", لیکه);
#د_که_پای

	لكل (ماشوم = لیکه->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
		بل_ماشوم = ماشوم->بل;

		بدل (ماشوم->type) {
		حالة BOX_جدول_CELL:
			/* ok */
			إذا (box_normalise_بلاک(ماشوم, ریښه, c) == false)
				أرجع false;
			cell = ماشوم;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_لیکه_ډله:
		حالة BOX_جدول_لیکه:
			/* insert implied table cell */
			أكد(لیکه->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					لیکه->style);
			إذا (style == عدم)
				أرجع false;

			cell = box_create(عدم, style, true, لیکه->href,
					لیکه->موخه, عدم, عدم, c->bctx);
			إذا (cell == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			cell->type = BOX_جدول_CELL;

			إذا (ماشوم->prev == عدم)
				لیکه->children = cell;
			وإلا
				ماشوم->prev->بل = cell;

			cell->prev = ماشوم->prev;

			طالما (ماشوم != عدم && (
					ماشوم->type == BOX_FLEX ||
					ماشوم->type == BOX_بلاک ||
					ماشوم->type == BOX_INLINE_CONTAINER ||
					ماشوم->type == BOX_جدول ||
					ماشوم->type == BOX_جدول_لیکه_ډله ||
					ماشوم->type == BOX_جدول_لیکه)) {
				box_جمع_کړه_ماشوم(cell, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			أكد(cell->وروستی != عدم);

			cell->وروستی->بل = عدم;
			cell->بل = بل_ماشوم = ماشوم;
			إذا (cell->بل != عدم)
				cell->بل->prev = cell;
			وإلا
				لیکه->وروستی = cell;
			cell->پلار = لیکه;

			إذا (box_normalise_بلاک(cell, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}

		إذا (calculate_جدول_لیکه(ستون_info, cell->columns, cell->لیکې,
				&cell->پیل_ستنه, cell) == false)
			أرجع false;
	}


	/* Update row spanning details for all columns */
	لكل (i = 0; i < ستون_info->عدد_columns; i++) {
		إذا (ستون_info->spans[i].لیکه_پراختیا != 0 &&
				ستون_info->spans[i].auto_لیکه == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			ستون_info->spans[i].لیکه_پراختیا--;
		}
	}

	/* Reset current column for next row */
	ستون_info->current_ستنه = 0;

	/* Increment row counter */
	ستون_info->عدد_لیکې++;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", لیکه);
#د_که_پای

	أرجع true;
}


ساكن bool
box_normalise_جدول_لیکه_ډله(بنية box *لیکه_ډله,
			      ثابت بنية box *ریښه,
			      بنية columns *ستون_info,
			      html_content * c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;
	بنية box *لیکه;
	css_computed_style *style;
	nscss_select_ctx ctx;
	بینښه صحيح ډله_لیکه_شمېر = 0;

	أكد(لیکه_ډله != 0);
	أكد(لیکه_ډله->type == BOX_جدول_لیکه_ډله);

	ctx.ریښه_style = ریښه->style;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", لیکه_ډله);
#د_که_پای

	لكل (ماشوم = لیکه_ډله->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
		بل_ماشوم = ماشوم->بل;

		بدل (ماشوم->type) {
		حالة BOX_جدول_لیکه:
			/* ok */
			ډله_لیکه_شمېر++;
			إذا (box_normalise_جدول_لیکه(ماشوم, ریښه, ستون_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_لیکه_ډله:
		حالة BOX_جدول_CELL:
			/* insert implied table row */
			أكد(لیکه_ډله->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					لیکه_ډله->style);
			إذا (style == عدم)
				أرجع false;

			لیکه = box_create(عدم, style, true, لیکه_ډله->href,
					لیکه_ډله->موخه, عدم, عدم, c->bctx);
			إذا (لیکه == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			لیکه->type = BOX_جدول_لیکه;

			إذا (ماشوم->prev == عدم)
				لیکه_ډله->children = لیکه;
			وإلا
				ماشوم->prev->بل = لیکه;

			لیکه->prev = ماشوم->prev;

			طالما (ماشوم != عدم && (
					ماشوم->type == BOX_FLEX ||
					ماشوم->type == BOX_بلاک ||
					ماشوم->type == BOX_INLINE_CONTAINER ||
					ماشوم->type == BOX_جدول ||
					ماشوم->type == BOX_جدول_لیکه_ډله ||
					ماشوم->type == BOX_جدول_CELL)) {
				box_جمع_کړه_ماشوم(لیکه, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			أكد(لیکه->وروستی != عدم);

			لیکه->وروستی->بل = عدم;
			لیکه->بل = بل_ماشوم = ماشوم;
			إذا (لیکه->بل != عدم)
				لیکه->بل->prev = لیکه;
			وإلا
				لیکه_ډله->وروستی = لیکه;
			لیکه->پلار = لیکه_ډله;

			ډله_لیکه_شمېر++;
			إذا (box_normalise_جدول_لیکه(لیکه, ریښه, ستون_info,
					c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
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

	إذا (لیکه_ډله->children == عدم) {
#که_تعریف_شوی BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#د_که_پای

		أكد(لیکه_ډله->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
		ctx.بنسټ_url = c->بنسټ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				لیکه_ډله->style);
		إذا (style == عدم) {
			أرجع false;
		}

		لیکه = box_create(عدم, style, true, لیکه_ډله->href,
				لیکه_ډله->موخه, عدم, عدم, c->bctx);
		إذا (لیکه == عدم) {
			css_computed_style_destroy(style);
			أرجع false;
		}
		لیکه->type = BOX_جدول_لیکه;

		لیکه->پلار = لیکه_ډله;
		لیکه_ډله->children = لیکه_ډله->وروستی = لیکه;

		ډله_لیکه_شمېر = 1;

		/* Keep table's row count in sync */
		ستون_info->عدد_لیکې++;
	}

	لیکه_ډله->لیکې = ډله_لیکه_شمېر;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", لیکه_ډله);
#د_که_پای

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
			  ثابت بنية box *ریښه,
			  بنية پراختیا_info *spans,
			  html_content *c)
{
	بنية box *جدول_لیکه_ډله;
	بنية box *جدول_لیکه;
	بنية box *جدول_cell;
	بینښه صحيح لیکې_کیڼ = جدول->لیکې;
	بینښه صحيح ډله_لیکې_کیڼ;
	بینښه صحيح ستون;
	nscss_select_ctx ctx;

	ctx.ریښه_style = ریښه->style;

	/* Clear span data */
	memset(spans, 0, جدول->columns * حجم(بنية پراختیا_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	لكل (جدول_لیکه_ډله = جدول->children;
	     جدول_لیکه_ډله != عدم;
	     جدول_لیکه_ډله = جدول_لیکه_ډله->بل) {

		ډله_لیکې_کیڼ = جدول_لیکه_ډله->لیکې;

		لكل (جدول_لیکه = جدول_لیکه_ډله->children;
		     جدول_لیکه != عدم;
		     جدول_لیکه = جدول_لیکه->بل) {

			لكل (جدول_cell = جدول_لیکه->children;
			     جدول_cell != عدم;
			     جدول_cell = جدول_cell->بل) {

				/* colspan = 0 -> colspan = 1 */
				إذا (جدول_cell->columns == 0) {
					جدول_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				إذا (جدول_cell->لیکې == 0) {
					جدول_cell->لیکې = ډله_لیکې_کیڼ;
				}

				/* limit rowspans within group */
				إذا (جدول_cell->لیکې > ډله_لیکې_کیڼ) {
					جدول_cell->لیکې = ډله_لیکې_کیڼ;
				}

				/* Record span information */
				لكل (ستون = جدول_cell->پیل_ستنه;
						ستون < جدول_cell->پیل_ستنه +
						جدول_cell->columns; ستون++) {
					spans[ستون].لیکه_پراختیا = جدول_cell->لیکې;
				}
			}

			/* Reduce span count of each column */
			لكل (ستون = 0; ستون < جدول->columns; ستون++) {
				إذا (spans[ستون].لیکه_پراختیا == 0) {
					بینښه صحيح پیل = ستون;
					css_computed_style *style;
					بنية box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(جدول_لیکه->style != عدم);

					/* Find width of gap */
					طالما (ستون < جدول->columns &&
							spans[ستون].لیکه_پراختیا ==
							0) {
						ستون++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
					ctx.بنسټ_url = c->بنسټ_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							جدول_لیکه->style);
					إذا (style == عدم)
						أرجع false;

					cell = box_create(عدم, style, true,
							جدول_لیکه->href,
							جدول_لیکه->موخه,
							عدم, عدم, c->bctx);
					إذا (cell == عدم) {
						css_computed_style_destroy(
								style);
						أرجع false;
					}
					cell->type = BOX_جدول_CELL;

					cell->لیکې = 1;
					cell->columns = ستون - پیل;
					cell->پیل_ستنه = پیل;

					/* Find place to insert cell */
					لكل (prev = جدول_لیکه->children;
							prev != عدم;
							prev = prev->بل) {
						إذا (prev->پیل_ستنه +
							prev->columns ==
								پیل)
							اكسر;
						إذا (prev->بل == عدم)
							اكسر;
					}

					/* Insert it */
					إذا (prev == عدم) {
						إذا (جدول_لیکه->children != عدم)
							جدول_لیکه->children->
								prev = cell;
						وإلا
							جدول_لیکه->وروستی = cell;

						cell->بل =
							جدول_لیکه->children;
						جدول_لیکه->children = cell;
					} وإلا {
						إذا (prev->بل != عدم)
							prev->بل->prev = cell;
						وإلا
							جدول_لیکه->وروستی = cell;

						cell->بل = prev->بل;
						prev->بل = cell;
						cell->prev = prev;
					}
					cell->پلار = جدول_لیکه;
				} وإلا {
					spans[ستون].لیکه_پراختیا--;
				}
			}

			أكد(لیکې_کیڼ > 0);

			لیکې_کیڼ--;
		}

		ډله_لیکې_کیڼ--;
	}

	أرجع true;
}


ساكن bool
box_normalise_جدول(بنية box *جدول, ثابت بنية box *ریښه, html_content * c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;
	بنية box *لیکه_ډله;
	css_computed_style *style;
	بنية columns ستون_info;
	nscss_select_ctx ctx;

	أكد(جدول != عدم);
	أكد(جدول->type == BOX_جدول);

	ctx.ریښه_style = ریښه->style;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", جدول);
#د_که_پای

	ستون_info.عدد_columns = 1;
	ستون_info.current_ستنه = 0;
	ستون_info.spans = حافظه_ورکړه(2 * حجم *ستون_info.spans);
	إذا (ستون_info.spans == عدم)
		أرجع false;

	ستون_info.spans[0].لیکه_پراختیا = ستون_info.spans[1].لیکه_پراختیا = 0;
	ستون_info.spans[0].auto_لیکه = false;
	ستون_info.spans[1].auto_لیکه = false;
	ستون_info.عدد_لیکې = 0;

	لكل (ماشوم = جدول->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
		بل_ماشوم = ماشوم->بل;
		بدل (ماشوم->type) {
		حالة BOX_جدول_لیکه_ډله:
			/* ok */
			إذا (box_normalise_جدول_لیکه_ډله(ماشوم, ریښه,
					&ستون_info, c) == false) {
				حرر(ستون_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_لیکه:
		حالة BOX_جدول_CELL:
			/* insert implied table row group */
			أكد(جدول->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					جدول->style);
			إذا (style == عدم) {
				حرر(ستون_info.spans);
				أرجع false;
			}

			لیکه_ډله = box_create(عدم, style, true, جدول->href,
					جدول->موخه, عدم, عدم, c->bctx);
			إذا (لیکه_ډله == عدم) {
				css_computed_style_destroy(style);
				حرر(ستون_info.spans);
				أرجع false;
			}

			لیکه_ډله->type = BOX_جدول_لیکه_ډله;

			إذا (ماشوم->prev == عدم)
				جدول->children = لیکه_ډله;
			وإلا
				ماشوم->prev->بل = لیکه_ډله;

			لیکه_ډله->prev = ماشوم->prev;

			طالما (ماشوم != عدم && (
					ماشوم->type == BOX_FLEX ||
					ماشوم->type == BOX_بلاک ||
					ماشوم->type == BOX_INLINE_CONTAINER ||
					ماشوم->type == BOX_جدول ||
					ماشوم->type == BOX_جدول_لیکه ||
					ماشوم->type == BOX_جدول_CELL)) {
				box_جمع_کړه_ماشوم(لیکه_ډله, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			أكد(لیکه_ډله->وروستی != عدم);

			لیکه_ډله->وروستی->بل = عدم;
			لیکه_ډله->بل = بل_ماشوم = ماشوم;
			إذا (لیکه_ډله->بل != عدم)
				لیکه_ډله->بل->prev = لیکه_ډله;
			وإلا
				جدول->وروستی = لیکه_ډله;
			لیکه_ډله->پلار = جدول;

			إذا (box_normalise_جدول_لیکه_ډله(لیکه_ډله, ریښه,
					&ستون_info, c) == false) {
				حرر(ستون_info.spans);
				أرجع false;
			}
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			fprintf(stderr, "%i\n", ماشوم->type);
			أكد(0);
		}
	}

	جدول->columns = ستون_info.عدد_columns;
	جدول->لیکې = ستون_info.عدد_لیکې;

	إذا (جدول->children == عدم) {
		بنية box *لیکه;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#د_که_پای

		أكد(جدول->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
		ctx.بنسټ_url = c->بنسټ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				جدول->style);
		إذا (style == عدم) {
			حرر(ستون_info.spans);
			أرجع false;
		}

		لیکه_ډله = box_create(عدم, style, true, جدول->href,
				جدول->موخه, عدم, عدم, c->bctx);
		إذا (لیکه_ډله == عدم) {
			css_computed_style_destroy(style);
			حرر(ستون_info.spans);
			أرجع false;
		}
		لیکه_ډله->type = BOX_جدول_لیکه_ډله;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				لیکه_ډله->style);
		إذا (style == عدم) {
			box_free(لیکه_ډله);
			حرر(ستون_info.spans);
			أرجع false;
		}

		لیکه = box_create(عدم, style, true, لیکه_ډله->href,
				لیکه_ډله->موخه, عدم, عدم, c->bctx);
		إذا (لیکه == عدم) {
			css_computed_style_destroy(style);
			box_free(لیکه_ډله);
			حرر(ستون_info.spans);
			أرجع false;
		}
		لیکه->type = BOX_جدول_لیکه;

		لیکه->پلار = لیکه_ډله;
		لیکه_ډله->children = لیکه_ډله->وروستی = لیکه;

		لیکه_ډله->پلار = جدول;
		جدول->children = جدول->وروستی = لیکه_ډله;

		جدول->لیکې = 1;
	}

	إذا (box_normalise_جدول_spans(جدول, ریښه, ستون_info.spans, c) == false) {
		حرر(ستون_info.spans);
		أرجع false;
	}

	حرر(ستون_info.spans);

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", جدول);
#د_که_پای

	أرجع true;
}

ساكن bool box_normalise_flex(
		بنية box *flex_container,
		ثابت بنية box *ریښه,
		html_content *c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;
	بنية box *implied_flex_توکی;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != عدم);
	أكد(ریښه != عدم);

	ctx.ریښه_style = ریښه->style;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#د_که_پای

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	لكل (ماشوم = flex_container->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
#که_تعریف_شوی BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				ماشوم, ماشوم->type);
#د_که_پای

		بل_ماشوم = ماشوم->بل;	/* child may be destroyed */

		بدل (ماشوم->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_توکی = box_create(عدم, style, true,
					flex_container->href,
					flex_container->موخه,
					عدم, عدم, c->bctx);
			إذا (implied_flex_توکی == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_توکی->type = BOX_بلاک;

			إذا (ماشوم->prev == عدم)
				flex_container->children = implied_flex_توکی;
			وإلا
				ماشوم->prev->بل = implied_flex_توکی;

			implied_flex_توکی->prev = ماشوم->prev;

			طالما (ماشوم != عدم &&
					ماشوم->type == BOX_INLINE_CONTAINER) {
				box_جمع_کړه_ماشوم(implied_flex_توکی, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			implied_flex_توکی->وروستی->بل = عدم;
			implied_flex_توکی->بل = بل_ماشوم = ماشوم;
			إذا (implied_flex_توکی->بل != عدم)
				implied_flex_توکی->بل->prev = implied_flex_توکی;
			وإلا
				flex_container->وروستی = implied_flex_توکی;
			implied_flex_توکی->پلار = flex_container;

			إذا (box_normalise_بلاک(implied_flex_توکی,
					ریښه, c) == false)
				أرجع false;
			اكسر;

		حالة BOX_جدول:
			إذا (box_normalise_جدول(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_لیکه_ډله:
		حالة BOX_جدول_لیکه:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				أرجع false;

			implied_flex_توکی = box_create(عدم, style, true,
					flex_container->href,
					flex_container->موخه,
					عدم, عدم, c->bctx);
			إذا (implied_flex_توکی == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			implied_flex_توکی->type = BOX_جدول;

			إذا (ماشوم->prev == عدم)
				flex_container->children = implied_flex_توکی;
			وإلا
				ماشوم->prev->بل = implied_flex_توکی;

			implied_flex_توکی->prev = ماشوم->prev;

			طالما (ماشوم != عدم && (
					ماشوم->type == BOX_جدول_لیکه_ډله ||
					ماشوم->type == BOX_جدول_لیکه ||
					ماشوم->type == BOX_جدول_CELL)) {
				box_جمع_کړه_ماشوم(implied_flex_توکی, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			implied_flex_توکی->وروستی->بل = عدم;
			implied_flex_توکی->بل = بل_ماشوم = ماشوم;
			إذا (implied_flex_توکی->بل != عدم)
				implied_flex_توکی->بل->prev = implied_flex_توکی;
			وإلا
				flex_container->وروستی = implied_flex_توکی;
			implied_flex_توکی->پلار = flex_container;

			إذا (box_normalise_جدول(implied_flex_توکی,
					ریښه, c) == false)
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
			       ثابت بنية box *ریښه,
			       html_content * c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;

	أكد(cont != عدم);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#د_که_پای

	لكل (ماشوم = cont->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
		بل_ماشوم = ماشوم->بل;
		بدل (ماشوم->type) {
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_BR:
		حالة BOX_متن:
			/* ok */
			اكسر;
		حالة BOX_INLINE_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_FLEX:
			/* ok */
			إذا (box_normalise_flex(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
			/* ok */
			أكد(ماشوم->children != عدم);

			بدل (ماشوم->children->type) {
			حالة BOX_بلاک:
				إذا (box_normalise_بلاک(ماشوم->children, ریښه,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_جدول:
				إذا (box_normalise_جدول(ماشوم->children, ریښه,
						c) == false)
					أرجع false;
				اكسر;
			حالة BOX_FLEX:
				إذا (box_normalise_flex(ماشوم->children, ریښه,
						c) == false)
					أرجع false;
				اكسر;
			افتراضي:
				أكد(0);
			}

			إذا (ماشوم->children == عدم) {
				/* the child has destroyed itself: remove float */
				إذا (ماشوم->prev == عدم)
					ماشوم->پلار->children = ماشوم->بل;
				وإلا
					ماشوم->prev->بل = ماشوم->بل;
				إذا (ماشوم->بل != عدم)
					ماشوم->بل->prev = ماشوم->prev;
				وإلا
					ماشوم->پلار->وروستی = ماشوم->prev;

				box_free(ماشوم);
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_بلاک:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_جدول:
		حالة BOX_جدول_لیکه_ډله:
		حالة BOX_جدول_لیکه:
		حالة BOX_جدول_CELL:
		افتراضي:
			أكد(0);
		}
	}

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#د_که_پای

	أرجع true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_بلاک(بنية box *بلاک, ثابت بنية box *ریښه, html_content *c)
{
	بنية box *ماشوم;
	بنية box *بل_ماشوم;
	بنية box *جدول;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(بلاک != عدم);
	أكد(ریښه != عدم);

	ctx.ریښه_style = ریښه->style;

#که_تعریف_شوی BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", بلاک, بلاک->type);
#د_که_پای

	أكد(بلاک->type == BOX_بلاک || بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	لكل (ماشوم = بلاک->children; ماشوم != عدم; ماشوم = بل_ماشوم) {
#که_تعریف_شوی BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", ماشوم,
		      ماشوم->type);
#د_که_پای

		بل_ماشوم = ماشوم->بل;	/* child may be destroyed */

		بدل (ماشوم->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_بلاک:
			/* ok */
			إذا (box_normalise_بلاک(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			إذا (box_normalise_inline_container(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_جدول:
			إذا (box_normalise_جدول(ماشوم, ریښه, c) == false)
				أرجع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_پای_ټکی:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_بلاک:
		حالة BOX_FLOAT_کیڼ:
		حالة BOX_FLOAT_ښی:
		حالة BOX_BR:
		حالة BOX_متن:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_جدول_لیکه_ډله:
		حالة BOX_جدول_لیکه:
		حالة BOX_جدول_CELL:
			/* insert implied table */
			أكد(بلاک->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک);
			ctx.بنسټ_url = c->بنسټ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					بلاک->style);
			إذا (style == عدم)
				أرجع false;

			جدول = box_create(عدم, style, true, بلاک->href,
					بلاک->موخه, عدم, عدم, c->bctx);
			إذا (جدول == عدم) {
				css_computed_style_destroy(style);
				أرجع false;
			}
			جدول->type = BOX_جدول;

			إذا (ماشوم->prev == عدم)
				بلاک->children = جدول;
			وإلا
				ماشوم->prev->بل = جدول;

			جدول->prev = ماشوم->prev;

			طالما (ماشوم != عدم && (
					ماشوم->type == BOX_جدول_لیکه_ډله ||
					ماشوم->type == BOX_جدول_لیکه ||
					ماشوم->type == BOX_جدول_CELL)) {
				box_جمع_کړه_ماشوم(جدول, ماشوم);

				بل_ماشوم = ماشوم->بل;
				ماشوم->بل = عدم;
				ماشوم = بل_ماشوم;
			}

			جدول->وروستی->بل = عدم;
			جدول->بل = بل_ماشوم = ماشوم;
			إذا (جدول->بل != عدم)
				جدول->بل->prev = جدول;
			وإلا
				بلاک->وروستی = جدول;
			جدول->پلار = بلاک;

			إذا (box_normalise_جدول(جدول, ریښه, c) == false)
				أرجع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	أرجع true;
}
