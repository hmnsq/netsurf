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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "css/select.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/table.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_manipulate.h"
#అన్తర్భావ "html/box_normalise.h"

/* Define to enable box normalise debug */
#నిర్వచనం_రద్దు BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
నిర్మాణం పరిధి_info {
	/** Number of rows this cell spans */
	గుర్తులేని పూర్ణాంకం అడ్డువరుస_పరిధి;
	/** Row group of cell */
	నిర్మాణం box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_అడ్డువరుస;
};

/**
 * Column record for a table
 */
నిర్మాణం columns {
	/** Current column index */
	గుర్తులేని పూర్ణాంకం current_స్తంభం;
	/** Number of columns in main part of table 1..max columns */
	గుర్తులేని పూర్ణాంకం అంకె_columns;
	/** Information about columns in main table, array [0, num_columns) */
	నిర్మాణం పరిధి_info *spans;
	/** Number of rows in table */
	గుర్తులేని పూర్ణాంకం అంకె_అడ్డువరుసలు;
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
స్థిర bool
calculate_పట్టిక_అడ్డువరుస(నిర్మాణం columns *నిలువు_info,
		    గుర్తులేని పూర్ణాంకం నిలువు_పరిధి,
		    గుర్తులేని పూర్ణాంకం అడ్డువరుస_పరిధి,
		    గుర్తులేని పూర్ణాంకం *ప్రారంభం_స్తంభం,
		    నిర్మాణం box *cell)
{
	గుర్తులేని పూర్ణాంకం cell_ప్రారంభం_నిలువు = నిలువు_info->current_స్తంభం;
	గుర్తులేని పూర్ణాంకం cell_చివర_నిలువు;
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం పరిధి_info *spans;
	నిర్మాణం box *rg = cell->మాతృ->మాతృ; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	అయితే (నిలువు_info->spans[cell_ప్రారంభం_నిలువు].అడ్డువరుస_పరిధి != 0 &&
			నిలువు_info->spans[cell_ప్రారంభం_నిలువు].rg == rg) {
		cell_ప్రారంభం_నిలువు++;
	}

	/* Update current column with calculated start */
	నిలువు_info->current_స్తంభం = cell_ప్రారంభం_నిలువు;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	ఒకవేళ (నిలువు_పరిధి == 0)
		నిలువు_పరిధి = 1;

	cell_చివర_నిలువు = cell_ప్రారంభం_నిలువు + నిలువు_పరిధి;

	ఒకవేళ (నిలువు_info->అంకె_columns < cell_చివర_నిలువు) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(నిలువు_info->spans,
				పరిమాణం *spans * (cell_చివర_నిలువు + 1));
		ఒకవేళ (spans == ఖాళీ)
			తిరిగి false;

		నిలువు_info->spans = spans;
		నిలువు_info->అంకె_columns = cell_చివర_నిలువు;

		/* Mark new final column as sentinel */
		నిలువు_info->spans[cell_చివర_నిలువు].అడ్డువరుస_పరిధి = 0;
		నిలువు_info->spans[cell_చివర_నిలువు].auto_అడ్డువరుస = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	కొరకు (i = cell_ప్రారంభం_నిలువు; i < cell_చివర_నిలువు; i++) {
		నిలువు_info->spans[i].అడ్డువరుస_పరిధి = (అడ్డువరుస_పరిధి == 0) ? 1 : అడ్డువరుస_పరిధి;
		నిలువు_info->spans[i].auto_అడ్డువరుస = (అడ్డువరుస_పరిధి == 0);
		నిలువు_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	నిలువు_info->current_స్తంభం = cell_చివర_నిలువు;

	*ప్రారంభం_స్తంభం = cell_ప్రారంభం_నిలువు;

	తిరిగి true;
}


స్థిర bool
box_normalise_పట్టిక_అడ్డువరుస(నిర్మాణం box *అడ్డువరుస,
			స్థిరరాశి నిర్మాణం box *మూలం,
			నిర్మాణం columns *నిలువు_info,
			html_content * c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;
	నిర్మాణం box *cell = ఖాళీ;
	css_computed_style *style;
	గుర్తులేని పూర్ణాంకం i;
	nscss_select_ctx ctx;

	ప్రతిజ్ఞా(అడ్డువరుస != ఖాళీ);
	ప్రతిజ్ఞా(అడ్డువరుస->type == BOX_పట్టిక_అడ్డువరుస);

	ctx.మూలం_style = మూలం->style;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", అడ్డువరుస);
#అయితే_ముగింపు

	కొరకు (బిడ్డ = అడ్డువరుస->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;

		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_పట్టిక_CELL:
			/* ok */
			ఒకవేళ (box_normalise_ఖండం(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			cell = బిడ్డ;
			ఆపు;
		స్థితి BOX_FLEX:
		స్థితి BOX_ఖండం:
		స్థితి BOX_INLINE_CONTAINER:
		స్థితి BOX_పట్టిక:
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		స్థితి BOX_పట్టిక_అడ్డువరుస:
			/* insert implied table cell */
			ప్రతిజ్ఞా(అడ్డువరుస->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					అడ్డువరుస->style);
			ఒకవేళ (style == ఖాళీ)
				తిరిగి false;

			cell = box_create(ఖాళీ, style, true, అడ్డువరుస->href,
					అడ్డువరుస->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (cell == ఖాళీ) {
				css_computed_style_destroy(style);
				తిరిగి false;
			}
			cell->type = BOX_పట్టిక_CELL;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				అడ్డువరుస->children = cell;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = cell;

			cell->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ && (
					బిడ్డ->type == BOX_FLEX ||
					బిడ్డ->type == BOX_ఖండం ||
					బిడ్డ->type == BOX_INLINE_CONTAINER ||
					బిడ్డ->type == BOX_పట్టిక ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస_సమూహం ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస)) {
				box_కూడు_బిడ్డ(cell, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			ప్రతిజ్ఞా(cell->చివరి != ఖాళీ);

			cell->చివరి->తదుపరి = ఖాళీ;
			cell->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (cell->తదుపరి != ఖాళీ)
				cell->తదుపరి->prev = cell;
			లేకపోతే
				అడ్డువరుస->చివరి = cell;
			cell->మాతృ = అడ్డువరుస;

			ఒకవేళ (box_normalise_ఖండం(cell, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_INLINE_FLEX:
		స్థితి BOX_INLINE_ఖండం:
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ప్రతిజ్ఞా(0);
			ఆపు;
		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}

		ఒకవేళ (calculate_పట్టిక_అడ్డువరుస(నిలువు_info, cell->columns, cell->అడ్డువరుసలు,
				&cell->ప్రారంభం_స్తంభం, cell) == false)
			తిరిగి false;
	}


	/* Update row spanning details for all columns */
	కొరకు (i = 0; i < నిలువు_info->అంకె_columns; i++) {
		ఒకవేళ (నిలువు_info->spans[i].అడ్డువరుస_పరిధి != 0 &&
				నిలువు_info->spans[i].auto_అడ్డువరుస == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			నిలువు_info->spans[i].అడ్డువరుస_పరిధి--;
		}
	}

	/* Reset current column for next row */
	నిలువు_info->current_స్తంభం = 0;

	/* Increment row counter */
	నిలువు_info->అంకె_అడ్డువరుసలు++;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", అడ్డువరుస);
#అయితే_ముగింపు

	తిరిగి true;
}


స్థిర bool
box_normalise_పట్టిక_అడ్డువరుస_సమూహం(నిర్మాణం box *అడ్డువరుస_సమూహం,
			      స్థిరరాశి నిర్మాణం box *మూలం,
			      నిర్మాణం columns *నిలువు_info,
			      html_content * c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;
	నిర్మాణం box *అడ్డువరుస;
	css_computed_style *style;
	nscss_select_ctx ctx;
	గుర్తులేని పూర్ణాంకం సమూహం_అడ్డువరుస_లెక్క = 0;

	ప్రతిజ్ఞా(అడ్డువరుస_సమూహం != 0);
	ప్రతిజ్ఞా(అడ్డువరుస_సమూహం->type == BOX_పట్టిక_అడ్డువరుస_సమూహం);

	ctx.మూలం_style = మూలం->style;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", అడ్డువరుస_సమూహం);
#అయితే_ముగింపు

	కొరకు (బిడ్డ = అడ్డువరుస_సమూహం->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;

		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_పట్టిక_అడ్డువరుస:
			/* ok */
			సమూహం_అడ్డువరుస_లెక్క++;
			ఒకవేళ (box_normalise_పట్టిక_అడ్డువరుస(బిడ్డ, మూలం, నిలువు_info,
					c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_FLEX:
		స్థితి BOX_ఖండం:
		స్థితి BOX_INLINE_CONTAINER:
		స్థితి BOX_పట్టిక:
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		స్థితి BOX_పట్టిక_CELL:
			/* insert implied table row */
			ప్రతిజ్ఞా(అడ్డువరుస_సమూహం->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					అడ్డువరుస_సమూహం->style);
			ఒకవేళ (style == ఖాళీ)
				తిరిగి false;

			అడ్డువరుస = box_create(ఖాళీ, style, true, అడ్డువరుస_సమూహం->href,
					అడ్డువరుస_సమూహం->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (అడ్డువరుస == ఖాళీ) {
				css_computed_style_destroy(style);
				తిరిగి false;
			}
			అడ్డువరుస->type = BOX_పట్టిక_అడ్డువరుస;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				అడ్డువరుస_సమూహం->children = అడ్డువరుస;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = అడ్డువరుస;

			అడ్డువరుస->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ && (
					బిడ్డ->type == BOX_FLEX ||
					బిడ్డ->type == BOX_ఖండం ||
					బిడ్డ->type == BOX_INLINE_CONTAINER ||
					బిడ్డ->type == BOX_పట్టిక ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస_సమూహం ||
					బిడ్డ->type == BOX_పట్టిక_CELL)) {
				box_కూడు_బిడ్డ(అడ్డువరుస, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			ప్రతిజ్ఞా(అడ్డువరుస->చివరి != ఖాళీ);

			అడ్డువరుస->చివరి->తదుపరి = ఖాళీ;
			అడ్డువరుస->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (అడ్డువరుస->తదుపరి != ఖాళీ)
				అడ్డువరుస->తదుపరి->prev = అడ్డువరుస;
			లేకపోతే
				అడ్డువరుస_సమూహం->చివరి = అడ్డువరుస;
			అడ్డువరుస->మాతృ = అడ్డువరుస_సమూహం;

			సమూహం_అడ్డువరుస_లెక్క++;
			ఒకవేళ (box_normalise_పట్టిక_అడ్డువరుస(అడ్డువరుస, మూలం, నిలువు_info,
					c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_INLINE_FLEX:
		స్థితి BOX_INLINE_ఖండం:
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ప్రతిజ్ఞా(0);
			ఆపు;
		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}
	}

	ఒకవేళ (అడ్డువరుస_సమూహం->children == ఖాళీ) {
#నిర్వచితమైతే BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#అయితే_ముగింపు

		ప్రతిజ్ఞా(అడ్డువరుస_సమూహం->style != ఖాళీ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
		ctx.పునాది_url = c->పునాది_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				అడ్డువరుస_సమూహం->style);
		ఒకవేళ (style == ఖాళీ) {
			తిరిగి false;
		}

		అడ్డువరుస = box_create(ఖాళీ, style, true, అడ్డువరుస_సమూహం->href,
				అడ్డువరుస_సమూహం->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
		ఒకవేళ (అడ్డువరుస == ఖాళీ) {
			css_computed_style_destroy(style);
			తిరిగి false;
		}
		అడ్డువరుస->type = BOX_పట్టిక_అడ్డువరుస;

		అడ్డువరుస->మాతృ = అడ్డువరుస_సమూహం;
		అడ్డువరుస_సమూహం->children = అడ్డువరుస_సమూహం->చివరి = అడ్డువరుస;

		సమూహం_అడ్డువరుస_లెక్క = 1;

		/* Keep table's row count in sync */
		నిలువు_info->అంకె_అడ్డువరుసలు++;
	}

	అడ్డువరుస_సమూహం->అడ్డువరుసలు = సమూహం_అడ్డువరుస_లెక్క;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", అడ్డువరుస_సమూహం);
#అయితే_ముగింపు

	తిరిగి true;
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
స్థిర bool
box_normalise_పట్టిక_spans(నిర్మాణం box *పట్టిక,
			  స్థిరరాశి నిర్మాణం box *మూలం,
			  నిర్మాణం పరిధి_info *spans,
			  html_content *c)
{
	నిర్మాణం box *పట్టిక_అడ్డువరుస_సమూహం;
	నిర్మాణం box *పట్టిక_అడ్డువరుస;
	నిర్మాణం box *పట్టిక_cell;
	గుర్తులేని పూర్ణాంకం అడ్డువరుసలు_ఎడమ = పట్టిక->అడ్డువరుసలు;
	గుర్తులేని పూర్ణాంకం సమూహం_అడ్డువరుసలు_ఎడమ;
	గుర్తులేని పూర్ణాంకం నిలువు;
	nscss_select_ctx ctx;

	ctx.మూలం_style = మూలం->style;

	/* Clear span data */
	memset(spans, 0, పట్టిక->columns * పరిమాణం(నిర్మాణం పరిధి_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	కొరకు (పట్టిక_అడ్డువరుస_సమూహం = పట్టిక->children;
	     పట్టిక_అడ్డువరుస_సమూహం != ఖాళీ;
	     పట్టిక_అడ్డువరుస_సమూహం = పట్టిక_అడ్డువరుస_సమూహం->తదుపరి) {

		సమూహం_అడ్డువరుసలు_ఎడమ = పట్టిక_అడ్డువరుస_సమూహం->అడ్డువరుసలు;

		కొరకు (పట్టిక_అడ్డువరుస = పట్టిక_అడ్డువరుస_సమూహం->children;
		     పట్టిక_అడ్డువరుస != ఖాళీ;
		     పట్టిక_అడ్డువరుస = పట్టిక_అడ్డువరుస->తదుపరి) {

			కొరకు (పట్టిక_cell = పట్టిక_అడ్డువరుస->children;
			     పట్టిక_cell != ఖాళీ;
			     పట్టిక_cell = పట్టిక_cell->తదుపరి) {

				/* colspan = 0 -> colspan = 1 */
				ఒకవేళ (పట్టిక_cell->columns == 0) {
					పట్టిక_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				ఒకవేళ (పట్టిక_cell->అడ్డువరుసలు == 0) {
					పట్టిక_cell->అడ్డువరుసలు = సమూహం_అడ్డువరుసలు_ఎడమ;
				}

				/* limit rowspans within group */
				ఒకవేళ (పట్టిక_cell->అడ్డువరుసలు > సమూహం_అడ్డువరుసలు_ఎడమ) {
					పట్టిక_cell->అడ్డువరుసలు = సమూహం_అడ్డువరుసలు_ఎడమ;
				}

				/* Record span information */
				కొరకు (నిలువు = పట్టిక_cell->ప్రారంభం_స్తంభం;
						నిలువు < పట్టిక_cell->ప్రారంభం_స్తంభం +
						పట్టిక_cell->columns; నిలువు++) {
					spans[నిలువు].అడ్డువరుస_పరిధి = పట్టిక_cell->అడ్డువరుసలు;
				}
			}

			/* Reduce span count of each column */
			కొరకు (నిలువు = 0; నిలువు < పట్టిక->columns; నిలువు++) {
				ఒకవేళ (spans[నిలువు].అడ్డువరుస_పరిధి == 0) {
					గుర్తులేని పూర్ణాంకం ప్రారంభం = నిలువు;
					css_computed_style *style;
					నిర్మాణం box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ప్రతిజ్ఞా(పట్టిక_అడ్డువరుస->style != ఖాళీ);

					/* Find width of gap */
					అయితే (నిలువు < పట్టిక->columns &&
							spans[నిలువు].అడ్డువరుస_పరిధి ==
							0) {
						నిలువు++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
					ctx.పునాది_url = c->పునాది_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							పట్టిక_అడ్డువరుస->style);
					ఒకవేళ (style == ఖాళీ)
						తిరిగి false;

					cell = box_create(ఖాళీ, style, true,
							పట్టిక_అడ్డువరుస->href,
							పట్టిక_అడ్డువరుస->లక్ష్యం,
							ఖాళీ, ఖాళీ, c->bctx);
					ఒకవేళ (cell == ఖాళీ) {
						css_computed_style_destroy(
								style);
						తిరిగి false;
					}
					cell->type = BOX_పట్టిక_CELL;

					cell->అడ్డువరుసలు = 1;
					cell->columns = నిలువు - ప్రారంభం;
					cell->ప్రారంభం_స్తంభం = ప్రారంభం;

					/* Find place to insert cell */
					కొరకు (prev = పట్టిక_అడ్డువరుస->children;
							prev != ఖాళీ;
							prev = prev->తదుపరి) {
						ఒకవేళ (prev->ప్రారంభం_స్తంభం +
							prev->columns ==
								ప్రారంభం)
							ఆపు;
						ఒకవేళ (prev->తదుపరి == ఖాళీ)
							ఆపు;
					}

					/* Insert it */
					ఒకవేళ (prev == ఖాళీ) {
						ఒకవేళ (పట్టిక_అడ్డువరుస->children != ఖాళీ)
							పట్టిక_అడ్డువరుస->children->
								prev = cell;
						లేకపోతే
							పట్టిక_అడ్డువరుస->చివరి = cell;

						cell->తదుపరి =
							పట్టిక_అడ్డువరుస->children;
						పట్టిక_అడ్డువరుస->children = cell;
					} లేకపోతే {
						ఒకవేళ (prev->తదుపరి != ఖాళీ)
							prev->తదుపరి->prev = cell;
						లేకపోతే
							పట్టిక_అడ్డువరుస->చివరి = cell;

						cell->తదుపరి = prev->తదుపరి;
						prev->తదుపరి = cell;
						cell->prev = prev;
					}
					cell->మాతృ = పట్టిక_అడ్డువరుస;
				} లేకపోతే {
					spans[నిలువు].అడ్డువరుస_పరిధి--;
				}
			}

			ప్రతిజ్ఞా(అడ్డువరుసలు_ఎడమ > 0);

			అడ్డువరుసలు_ఎడమ--;
		}

		సమూహం_అడ్డువరుసలు_ఎడమ--;
	}

	తిరిగి true;
}


స్థిర bool
box_normalise_పట్టిక(నిర్మాణం box *పట్టిక, స్థిరరాశి నిర్మాణం box *మూలం, html_content * c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;
	నిర్మాణం box *అడ్డువరుస_సమూహం;
	css_computed_style *style;
	నిర్మాణం columns నిలువు_info;
	nscss_select_ctx ctx;

	ప్రతిజ్ఞా(పట్టిక != ఖాళీ);
	ప్రతిజ్ఞా(పట్టిక->type == BOX_పట్టిక);

	ctx.మూలం_style = మూలం->style;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", పట్టిక);
#అయితే_ముగింపు

	నిలువు_info.అంకె_columns = 1;
	నిలువు_info.current_స్తంభం = 0;
	నిలువు_info.spans = జ్ఞాపకం_ఇవ్వు(2 * పరిమాణం *నిలువు_info.spans);
	ఒకవేళ (నిలువు_info.spans == ఖాళీ)
		తిరిగి false;

	నిలువు_info.spans[0].అడ్డువరుస_పరిధి = నిలువు_info.spans[1].అడ్డువరుస_పరిధి = 0;
	నిలువు_info.spans[0].auto_అడ్డువరుస = false;
	నిలువు_info.spans[1].auto_అడ్డువరుస = false;
	నిలువు_info.అంకె_అడ్డువరుసలు = 0;

	కొరకు (బిడ్డ = పట్టిక->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
			/* ok */
			ఒకవేళ (box_normalise_పట్టిక_అడ్డువరుస_సమూహం(బిడ్డ, మూలం,
					&నిలువు_info, c) == false) {
				విడుదల(నిలువు_info.spans);
				తిరిగి false;
			}
			ఆపు;
		స్థితి BOX_FLEX:
		స్థితి BOX_ఖండం:
		స్థితి BOX_INLINE_CONTAINER:
		స్థితి BOX_పట్టిక:
		స్థితి BOX_పట్టిక_అడ్డువరుస:
		స్థితి BOX_పట్టిక_CELL:
			/* insert implied table row group */
			ప్రతిజ్ఞా(పట్టిక->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					పట్టిక->style);
			ఒకవేళ (style == ఖాళీ) {
				విడుదల(నిలువు_info.spans);
				తిరిగి false;
			}

			అడ్డువరుస_సమూహం = box_create(ఖాళీ, style, true, పట్టిక->href,
					పట్టిక->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (అడ్డువరుస_సమూహం == ఖాళీ) {
				css_computed_style_destroy(style);
				విడుదల(నిలువు_info.spans);
				తిరిగి false;
			}

			అడ్డువరుస_సమూహం->type = BOX_పట్టిక_అడ్డువరుస_సమూహం;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				పట్టిక->children = అడ్డువరుస_సమూహం;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = అడ్డువరుస_సమూహం;

			అడ్డువరుస_సమూహం->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ && (
					బిడ్డ->type == BOX_FLEX ||
					బిడ్డ->type == BOX_ఖండం ||
					బిడ్డ->type == BOX_INLINE_CONTAINER ||
					బిడ్డ->type == BOX_పట్టిక ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస ||
					బిడ్డ->type == BOX_పట్టిక_CELL)) {
				box_కూడు_బిడ్డ(అడ్డువరుస_సమూహం, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			ప్రతిజ్ఞా(అడ్డువరుస_సమూహం->చివరి != ఖాళీ);

			అడ్డువరుస_సమూహం->చివరి->తదుపరి = ఖాళీ;
			అడ్డువరుస_సమూహం->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (అడ్డువరుస_సమూహం->తదుపరి != ఖాళీ)
				అడ్డువరుస_సమూహం->తదుపరి->prev = అడ్డువరుస_సమూహం;
			లేకపోతే
				పట్టిక->చివరి = అడ్డువరుస_సమూహం;
			అడ్డువరుస_సమూహం->మాతృ = పట్టిక;

			ఒకవేళ (box_normalise_పట్టిక_అడ్డువరుస_సమూహం(అడ్డువరుస_సమూహం, మూలం,
					&నిలువు_info, c) == false) {
				విడుదల(నిలువు_info.spans);
				తిరిగి false;
			}
			ఆపు;
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_INLINE_FLEX:
		స్థితి BOX_INLINE_ఖండం:
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ప్రతిజ్ఞా(0);
			ఆపు;
		అప్రమేయ:
			fprintf(stderr, "%i\n", బిడ్డ->type);
			ప్రతిజ్ఞా(0);
		}
	}

	పట్టిక->columns = నిలువు_info.అంకె_columns;
	పట్టిక->అడ్డువరుసలు = నిలువు_info.అంకె_అడ్డువరుసలు;

	ఒకవేళ (పట్టిక->children == ఖాళీ) {
		నిర్మాణం box *అడ్డువరుస;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#అయితే_ముగింపు

		ప్రతిజ్ఞా(పట్టిక->style != ఖాళీ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
		ctx.పునాది_url = c->పునాది_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				పట్టిక->style);
		ఒకవేళ (style == ఖాళీ) {
			విడుదల(నిలువు_info.spans);
			తిరిగి false;
		}

		అడ్డువరుస_సమూహం = box_create(ఖాళీ, style, true, పట్టిక->href,
				పట్టిక->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
		ఒకవేళ (అడ్డువరుస_సమూహం == ఖాళీ) {
			css_computed_style_destroy(style);
			విడుదల(నిలువు_info.spans);
			తిరిగి false;
		}
		అడ్డువరుస_సమూహం->type = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				అడ్డువరుస_సమూహం->style);
		ఒకవేళ (style == ఖాళీ) {
			box_free(అడ్డువరుస_సమూహం);
			విడుదల(నిలువు_info.spans);
			తిరిగి false;
		}

		అడ్డువరుస = box_create(ఖాళీ, style, true, అడ్డువరుస_సమూహం->href,
				అడ్డువరుస_సమూహం->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
		ఒకవేళ (అడ్డువరుస == ఖాళీ) {
			css_computed_style_destroy(style);
			box_free(అడ్డువరుస_సమూహం);
			విడుదల(నిలువు_info.spans);
			తిరిగి false;
		}
		అడ్డువరుస->type = BOX_పట్టిక_అడ్డువరుస;

		అడ్డువరుస->మాతృ = అడ్డువరుస_సమూహం;
		అడ్డువరుస_సమూహం->children = అడ్డువరుస_సమూహం->చివరి = అడ్డువరుస;

		అడ్డువరుస_సమూహం->మాతృ = పట్టిక;
		పట్టిక->children = పట్టిక->చివరి = అడ్డువరుస_సమూహం;

		పట్టిక->అడ్డువరుసలు = 1;
	}

	ఒకవేళ (box_normalise_పట్టిక_spans(పట్టిక, మూలం, నిలువు_info.spans, c) == false) {
		విడుదల(నిలువు_info.spans);
		తిరిగి false;
	}

	విడుదల(నిలువు_info.spans);

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", పట్టిక);
#అయితే_ముగింపు

	తిరిగి true;
}

స్థిర bool box_normalise_flex(
		నిర్మాణం box *flex_container,
		స్థిరరాశి నిర్మాణం box *మూలం,
		html_content *c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;
	నిర్మాణం box *implied_flex_అంశం;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ప్రతిజ్ఞా(flex_container != ఖాళీ);
	ప్రతిజ్ఞా(మూలం != ఖాళీ);

	ctx.మూలం_style = మూలం->style;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#అయితే_ముగింపు

	ప్రతిజ్ఞా(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	కొరకు (బిడ్డ = flex_container->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
#నిర్వచితమైతే BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				బిడ్డ, బిడ్డ->type);
#అయితే_ముగింపు

		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;	/* child may be destroyed */

		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_FLEX:
			/* ok */
			ఒకవేళ (box_normalise_flex(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_ఖండం:
			/* ok */
			ఒకవేళ (box_normalise_ఖండం(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ప్రతిజ్ఞా(flex_container->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ఒకవేళ (style == ఖాళీ)
				తిరిగి false;

			implied_flex_అంశం = box_create(ఖాళీ, style, true,
					flex_container->href,
					flex_container->లక్ష్యం,
					ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (implied_flex_అంశం == ఖాళీ) {
				css_computed_style_destroy(style);
				తిరిగి false;
			}
			implied_flex_అంశం->type = BOX_ఖండం;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				flex_container->children = implied_flex_అంశం;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = implied_flex_అంశం;

			implied_flex_అంశం->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ &&
					బిడ్డ->type == BOX_INLINE_CONTAINER) {
				box_కూడు_బిడ్డ(implied_flex_అంశం, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			implied_flex_అంశం->చివరి->తదుపరి = ఖాళీ;
			implied_flex_అంశం->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (implied_flex_అంశం->తదుపరి != ఖాళీ)
				implied_flex_అంశం->తదుపరి->prev = implied_flex_అంశం;
			లేకపోతే
				flex_container->చివరి = implied_flex_అంశం;
			implied_flex_అంశం->మాతృ = flex_container;

			ఒకవేళ (box_normalise_ఖండం(implied_flex_అంశం,
					మూలం, c) == false)
				తిరిగి false;
			ఆపు;

		స్థితి BOX_పట్టిక:
			ఒకవేళ (box_normalise_పట్టిక(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_INLINE_FLEX:
		స్థితి BOX_INLINE_ఖండం:
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ప్రతిజ్ఞా(0);
			ఆపు;
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		స్థితి BOX_పట్టిక_అడ్డువరుస:
		స్థితి BOX_పట్టిక_CELL:
			/* insert implied table */
			ప్రతిజ్ఞా(flex_container->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ఒకవేళ (style == ఖాళీ)
				తిరిగి false;

			implied_flex_అంశం = box_create(ఖాళీ, style, true,
					flex_container->href,
					flex_container->లక్ష్యం,
					ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (implied_flex_అంశం == ఖాళీ) {
				css_computed_style_destroy(style);
				తిరిగి false;
			}
			implied_flex_అంశం->type = BOX_పట్టిక;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				flex_container->children = implied_flex_అంశం;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = implied_flex_అంశం;

			implied_flex_అంశం->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ && (
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస_సమూహం ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస ||
					బిడ్డ->type == BOX_పట్టిక_CELL)) {
				box_కూడు_బిడ్డ(implied_flex_అంశం, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			implied_flex_అంశం->చివరి->తదుపరి = ఖాళీ;
			implied_flex_అంశం->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (implied_flex_అంశం->తదుపరి != ఖాళీ)
				implied_flex_అంశం->తదుపరి->prev = implied_flex_అంశం;
			లేకపోతే
				flex_container->చివరి = implied_flex_అంశం;
			implied_flex_అంశం->మాతృ = flex_container;

			ఒకవేళ (box_normalise_పట్టిక(implied_flex_అంశం,
					మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}
	}

	తిరిగి true;
}

స్థిర bool
box_normalise_inline_container(నిర్మాణం box *cont,
			       స్థిరరాశి నిర్మాణం box *మూలం,
			       html_content * c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;

	ప్రతిజ్ఞా(cont != ఖాళీ);
	ప్రతిజ్ఞా(cont->type == BOX_INLINE_CONTAINER);

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#అయితే_ముగింపు

	కొరకు (బిడ్డ = cont->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* ok */
			ఆపు;
		స్థితి BOX_INLINE_ఖండం:
			/* ok */
			ఒకవేళ (box_normalise_ఖండం(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE_FLEX:
			/* ok */
			ఒకవేళ (box_normalise_flex(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
			/* ok */
			ప్రతిజ్ఞా(బిడ్డ->children != ఖాళీ);

			ఎంపిక (బిడ్డ->children->type) {
			స్థితి BOX_ఖండం:
				ఒకవేళ (box_normalise_ఖండం(బిడ్డ->children, మూలం,
						c) == false)
					తిరిగి false;
				ఆపు;
			స్థితి BOX_పట్టిక:
				ఒకవేళ (box_normalise_పట్టిక(బిడ్డ->children, మూలం,
						c) == false)
					తిరిగి false;
				ఆపు;
			స్థితి BOX_FLEX:
				ఒకవేళ (box_normalise_flex(బిడ్డ->children, మూలం,
						c) == false)
					తిరిగి false;
				ఆపు;
			అప్రమేయ:
				ప్రతిజ్ఞా(0);
			}

			ఒకవేళ (బిడ్డ->children == ఖాళీ) {
				/* the child has destroyed itself: remove float */
				ఒకవేళ (బిడ్డ->prev == ఖాళీ)
					బిడ్డ->మాతృ->children = బిడ్డ->తదుపరి;
				లేకపోతే
					బిడ్డ->prev->తదుపరి = బిడ్డ->తదుపరి;
				ఒకవేళ (బిడ్డ->తదుపరి != ఖాళీ)
					బిడ్డ->తదుపరి->prev = బిడ్డ->prev;
				లేకపోతే
					బిడ్డ->మాతృ->చివరి = బిడ్డ->prev;

				box_free(బిడ్డ);
			}
			ఆపు;
		స్థితి BOX_FLEX:
		స్థితి BOX_ఖండం:
		స్థితి BOX_INLINE_CONTAINER:
		స్థితి BOX_పట్టిక:
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		స్థితి BOX_పట్టిక_అడ్డువరుస:
		స్థితి BOX_పట్టిక_CELL:
		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}
	}

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#అయితే_ముగింపు

	తిరిగి true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_ఖండం(నిర్మాణం box *ఖండం, స్థిరరాశి నిర్మాణం box *మూలం, html_content *c)
{
	నిర్మాణం box *బిడ్డ;
	నిర్మాణం box *తదుపరి_బిడ్డ;
	నిర్మాణం box *పట్టిక;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ప్రతిజ్ఞా(ఖండం != ఖాళీ);
	ప్రతిజ్ఞా(మూలం != ఖాళీ);

	ctx.మూలం_style = మూలం->style;

#నిర్వచితమైతే BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", ఖండం, ఖండం->type);
#అయితే_ముగింపు

	ప్రతిజ్ఞా(ఖండం->type == BOX_ఖండం || ఖండం->type == BOX_INLINE_ఖండం ||
			ఖండం->type == BOX_పట్టిక_CELL);

	కొరకు (బిడ్డ = ఖండం->children; బిడ్డ != ఖాళీ; బిడ్డ = తదుపరి_బిడ్డ) {
#నిర్వచితమైతే BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", బిడ్డ,
		      బిడ్డ->type);
#అయితే_ముగింపు

		తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;	/* child may be destroyed */

		ఎంపిక (బిడ్డ->type) {
		స్థితి BOX_FLEX:
			/* ok */
			ఒకవేళ (box_normalise_flex(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_ఖండం:
			/* ok */
			ఒకవేళ (box_normalise_ఖండం(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE_CONTAINER:
			ఒకవేళ (box_normalise_inline_container(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_పట్టిక:
			ఒకవేళ (box_normalise_పట్టిక(బిడ్డ, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		స్థితి BOX_INLINE:
		స్థితి BOX_INLINE_చివర:
		స్థితి BOX_INLINE_FLEX:
		స్థితి BOX_INLINE_ఖండం:
		స్థితి BOX_FLOAT_ఎడమ:
		స్థితి BOX_FLOAT_కుడి:
		స్థితి BOX_BR:
		స్థితి BOX_పాఠ్యం:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ప్రతిజ్ఞా(0);
			ఆపు;
		స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		స్థితి BOX_పట్టిక_అడ్డువరుస:
		స్థితి BOX_పట్టిక_CELL:
			/* insert implied table */
			ప్రతిజ్ఞా(ఖండం->style != ఖాళీ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన);
			ctx.పునాది_url = c->పునాది_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ఖండం->style);
			ఒకవేళ (style == ఖాళీ)
				తిరిగి false;

			పట్టిక = box_create(ఖాళీ, style, true, ఖండం->href,
					ఖండం->లక్ష్యం, ఖాళీ, ఖాళీ, c->bctx);
			ఒకవేళ (పట్టిక == ఖాళీ) {
				css_computed_style_destroy(style);
				తిరిగి false;
			}
			పట్టిక->type = BOX_పట్టిక;

			ఒకవేళ (బిడ్డ->prev == ఖాళీ)
				ఖండం->children = పట్టిక;
			లేకపోతే
				బిడ్డ->prev->తదుపరి = పట్టిక;

			పట్టిక->prev = బిడ్డ->prev;

			అయితే (బిడ్డ != ఖాళీ && (
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస_సమూహం ||
					బిడ్డ->type == BOX_పట్టిక_అడ్డువరుస ||
					బిడ్డ->type == BOX_పట్టిక_CELL)) {
				box_కూడు_బిడ్డ(పట్టిక, బిడ్డ);

				తదుపరి_బిడ్డ = బిడ్డ->తదుపరి;
				బిడ్డ->తదుపరి = ఖాళీ;
				బిడ్డ = తదుపరి_బిడ్డ;
			}

			పట్టిక->చివరి->తదుపరి = ఖాళీ;
			పట్టిక->తదుపరి = తదుపరి_బిడ్డ = బిడ్డ;
			ఒకవేళ (పట్టిక->తదుపరి != ఖాళీ)
				పట్టిక->తదుపరి->prev = పట్టిక;
			లేకపోతే
				ఖండం->చివరి = పట్టిక;
			పట్టిక->మాతృ = ఖండం;

			ఒకవేళ (box_normalise_పట్టిక(పట్టిక, మూలం, c) == false)
				తిరిగి false;
			ఆపు;
		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}
	}

	తిరిగి true;
}
