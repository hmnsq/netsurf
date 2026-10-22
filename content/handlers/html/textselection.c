/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of text selection for a HTML content.
 */

#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "netsurf/plot_style.h"
#అన్తర్భావ "desktop/selection.h"
#అన్తర్భావ "desktop/save_text.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/textselection.h"

#నిర్వచించు SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

నిర్మాణం rdw_info {
	bool inited;
	నిర్మాణం rect r;
};


/**
 * Tests whether a text box lies partially within the given range of
 * byte offsets, returning the start and end indexes of the bytes
 * that are enclosed.
 *
 * \param box box to be tested
 * \param start_idx byte offset of start of range
 * \param end_idx byte offset of end of range
 * \param start_offset receives the start offset of the selected part
 * \param end_offset receives the end offset of the selected part
 * \return true iff the range encloses at least part of the box
 */
స్థిర bool
selected_part(నిర్మాణం box *box,
	      గుర్తులేని ప్రారంభం_idx,
	      గుర్తులేని చివర_idx,
	      గుర్తులేని *ప్రారంభం_స్థానభ్రంశం,
	      గుర్తులేని *చివర_స్థానభ్రంశం)
{
	size_t box_నిడివి = box->నిడివి + SPACE_LEN(box);

	ఒకవేళ (box_నిడివి > 0) {
		ఒకవేళ ((box->byte_స్థానభ్రంశం >= ప్రారంభం_idx) &&
		    (box->byte_స్థానభ్రంశం + box_నిడివి <= చివర_idx)) {

			/* fully enclosed */
			*ప్రారంభం_స్థానభ్రంశం = 0;
			*చివర_స్థానభ్రంశం = box_నిడివి;
			తిరిగి true;
		} లేకపోతే ఒకవేళ ((box->byte_స్థానభ్రంశం + box_నిడివి > ప్రారంభం_idx) &&
			   (box->byte_స్థానభ్రంశం < చివర_idx)) {
			/* partly enclosed */
			పూర్ణాంకం స్థానభ్రంశం = 0;
			పూర్ణాంకం len;

			ఒకవేళ (box->byte_స్థానభ్రంశం < ప్రారంభం_idx) {
				స్థానభ్రంశం = ప్రారంభం_idx - box->byte_స్థానభ్రంశం;
			}

			len = box_నిడివి - స్థానభ్రంశం;

			ఒకవేళ (box->byte_స్థానభ్రంశం + box_నిడివి > చివర_idx) {
				len = చివర_idx - (box->byte_స్థానభ్రంశం + స్థానభ్రంశం);
			}

			*ప్రారంభం_స్థానభ్రంశం = స్థానభ్రంశం;
			*చివర_స్థానభ్రంశం = స్థానభ్రంశం + len;

			తిరిగి true;
		}
	}
	తిరిగి false;
}


/**
 * Traverse the given box subtree adding the boxes inside the
 *   selection to the coordinate range.
 *
 * \param box box subtree
 * \param start_idx start of range within textual representation (bytes)
 * \param end_idx end of range
 * \param rdwi redraw range to fill in
 * \param do_marker whether deal enter any marker box
 * \return NSERROR_OK on success else error code
 */
స్థిర nserror
coords_from_range(నిర్మాణం box *box,
		  గుర్తులేని ప్రారంభం_idx,
		  గుర్తులేని చివర_idx,
		  నిర్మాణం rdw_info *rdwi,
		  bool do_marker)
{
	నిర్మాణం box *బిడ్డ;
	nserror res;

	ప్రతిజ్ఞా(box);

	/* If selection starts inside marker */
	ఒకవేళ (box->మాతృ &&
	    box->మాతృ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->మాతృ;
	}

	/* If box has a list marker */
	ఒకవేళ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					ప్రారంభం_idx,
					చివర_idx,
					rdwi,
					true);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ఒకవేళ (box->byte_స్థానభ్రంశం >= చివర_idx) {
		తిరిగి NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	బిడ్డ = box->children;

	ఒకవేళ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ఎడమ ||
	       box->type == BOX_FLOAT_కుడి) &&
	      !box->పాఠ్యం)) {
		గుర్తులేని ప్రారంభం_off;
		గుర్తులేని చివర_off;

		ఒకవేళ (selected_part(box, ప్రారంభం_idx, చివర_idx, &ప్రారంభం_off, &చివర_off)) {
			పూర్ణాంకం వెడల్పు, ఎత్తు;
			పూర్ణాంకం x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			వెడల్పు = box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2];
			ఎత్తు = box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM];

			ఒకవేళ ((box->type == BOX_పాఠ్యం) &&
			    (box->space != 0)) {
				వెడల్పు += box->space;
			}

			ఒకవేళ (rdwi->inited) {
				ఒకవేళ (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				ఒకవేళ (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				ఒకవేళ (x + వెడల్పు > rdwi->r.x1) {
					rdwi->r.x1 = x + వెడల్పు;
				}
				ఒకవేళ (y + ఎత్తు > rdwi->r.y1) {
					rdwi->r.y1 = y + ఎత్తు;
				}
			} లేకపోతే {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + వెడల్పు;
				rdwi->r.y1 = y + ఎత్తు;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ఒకవేళ (బిడ్డ) {
		నిర్మాణం box *తదుపరి = బిడ్డ->తదుపరి;

		అయితే (తదుపరి && తదుపరి->byte_స్థానభ్రంశం < ప్రారంభం_idx) {
			బిడ్డ = తదుపరి;
			తదుపరి = బిడ్డ->తదుపరి;
		}

		అయితే (బిడ్డ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			నిర్మాణం box *తదుపరి = బిడ్డ->తదుపరి;

			res = coords_from_range(బిడ్డ,
						ప్రారంభం_idx,
						చివర_idx,
						rdwi,
						false);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}

			బిడ్డ = తదుపరి;
		}
	}

	తిరిగి NSERROR_OK;
}


/**
 * Append the contents of a box to a selection along with style information
 *
 * \param text         pointer to text being added, or NULL for newline
 * \param length       length of text to be appended (bytes)
 * \param box          pointer to text box, or NULL if from textplain
 * \param unit_len_ctx      Length conversion context
 * \param handle       selection string to append to
 * \param whitespace_text    whitespace to place before text for formatting
 *                            may be NULL
 * \param whitespace_length  length of whitespace_text
 * \return NSERROR_OK iff successful and traversal should continue else error code
 */
స్థిర nserror
selection_copy_box(స్థిరరాశి అక్షరం *పాఠ్యం,
		   size_t నిడివి,
		   నిర్మాణం box *box,
		   స్థిరరాశి css_unit_ctx *unit_len_ctx,
		   నిర్మాణం selection_string *handle,
		   స్థిరరాశి అక్షరం *whitespace_పాఠ్యం,
		   size_t whitespace_నిడివి)
{
	bool కూడు_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = ఖాళీ;

	/* add any whitespace which precedes the text from this box */
	ఒకవేళ (whitespace_పాఠ్యం != ఖాళీ &&
	    whitespace_నిడివి > 0) {
		ఒకవేళ (!selection_string_append(whitespace_పాఠ్యం,
					     whitespace_నిడివి,
					     false,
					     pstyle,
					     handle)) {
			తిరిగి NSERROR_NOMEM;
		}
	}

	ఒకవేళ (box != ఖాళీ) {
		/* HTML */
		కూడు_space = (box->space != 0);

		ఒకవేళ (box->style != ఖాళీ) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} లేకపోతే {
			/* If there's no style, there must be no text */
			ప్రతిజ్ఞా(box->పాఠ్యం == ఖాళీ);
		}
	}

	/* add the text from this box */
	ఒకవేళ (!selection_string_append(పాఠ్యం, నిడివి, కూడు_space, pstyle, handle)) {
		తిరిగి NSERROR_NOMEM;
	}

	తిరిగి NSERROR_OK;
}


/**
 * Traverse the given box subtree, calling selection copy for all
 * boxes that lie (partially) within the given range
 *
 * \param box        box subtree
 * \param unit_len_ctx    Length conversion context.
 * \param start_idx  start of range within textual representation (bytes)
 * \param end_idx    end of range
 * \param handler    handler function to call
 * \param handle     handle to pass
 * \param before     type of whitespace to place before next encountered text
 * \param first      whether this is the first box with text
 * \param do_marker  whether deal enter any marker box
 * \return NSERROR_OK on sucess else error code
 */
స్థిర nserror
selection_copy(నిర్మాణం box *box,
	       స్థిరరాశి css_unit_ctx *unit_len_ctx,
	       గుర్తులేని ప్రారంభం_idx,
	       గుర్తులేని చివర_idx,
	       నిర్మాణం selection_string *selstr,
	       save_పాఠ్యం_whitespace *before,
	       bool *మొదటి,
	       bool do_marker)
{
	nserror res;
	నిర్మాణం box *బిడ్డ;
	స్థిరరాశి అక్షరం *whitespace_పాఠ్యం = "";
	size_t whitespace_నిడివి = 0;

	ప్రతిజ్ఞా(box);

	/* If selection starts inside marker */
	ఒకవేళ (box->మాతృ &&
	    box->మాతృ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->మాతృ;
	}

	/* If box has a list marker */
	ఒకవేళ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     ప్రారంభం_idx,
				     చివర_idx,
				     selstr,
				     before,
				     మొదటి,
				     true);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ఒకవేళ (box->byte_స్థానభ్రంశం >= చివర_idx) {
		తిరిగి NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	బిడ్డ = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	ఒకవేళ (before) {
		save_పాఠ్యం_solve_whitespace(box,
					   మొదటి,
					   before,
					   &whitespace_పాఠ్యం,
					   &whitespace_నిడివి);
	} లేకపోతే {
		whitespace_పాఠ్యం = ఖాళీ;
	}

	ఒకవేళ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ఎడమ ||
	       box->type == BOX_FLOAT_కుడి) &&
	      !box->పాఠ్యం)) {
		గుర్తులేని ప్రారంభం_off;
		గుర్తులేని చివర_off;

		ఒకవేళ (selected_part(box, ప్రారంభం_idx, చివర_idx, &ప్రారంభం_off, &చివర_off)) {
			res = selection_copy_box(box->పాఠ్యం + ప్రారంభం_off,
						 min(box->నిడివి, చివర_off) - ప్రారంభం_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_పాఠ్యం,
						 whitespace_నిడివి);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}
			ఒకవేళ (before) {
				*మొదటి = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ఒకవేళ (బిడ్డ) {
		నిర్మాణం box *తదుపరి = బిడ్డ->తదుపరి;

		అయితే (తదుపరి && తదుపరి->byte_స్థానభ్రంశం < ప్రారంభం_idx) {
			బిడ్డ = తదుపరి;
			తదుపరి = బిడ్డ->తదుపరి;
		}

		అయితే (బిడ్డ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			నిర్మాణం box *తదుపరి = బిడ్డ->తదుపరి;

			res = selection_copy(బిడ్డ,
					     unit_len_ctx,
					     ప్రారంభం_idx,
					     చివర_idx,
					     selstr,
					     before,
					     మొదటి,
					     false);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}

			బిడ్డ = తదుపరి;
		}
	}

	తిరిగి NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
స్థిర గుర్తులేని selection_label_subtree(నిర్మాణం box *box, గుర్తులేని idx)
{
	నిర్మాణం box *బిడ్డ;

	ప్రతిజ్ఞా(box != ఖాళీ);

	బిడ్డ = box->children;

	box->byte_స్థానభ్రంశం = idx;

	ఒకవేళ (box->పాఠ్యం) {
		idx += box->నిడివి + SPACE_LEN(box);
	}

	అయితే (బిడ్డ) {
		ఒకవేళ (బిడ్డ->list_marker) {
			idx = selection_label_subtree(బిడ్డ->list_marker, idx);
		}

		idx = selection_label_subtree(బిడ్డ, idx);
		బిడ్డ = బిడ్డ->తదుపరి;
	}

	తిరిగి idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(నిర్మాణం content *c,
			  గుర్తులేని ప్రారంభం_idx,
			  గుర్తులేని చివర_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	నిర్మాణం rdw_info rdw;

	ఒకవేళ (html->layout == ఖాళీ) {
		తిరిగి NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, ప్రారంభం_idx, చివర_idx, &rdw, false);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(నిర్మాణం content *c,
			గుర్తులేని ప్రారంభం_idx,
			గుర్తులేని చివర_idx,
			నిర్మాణం selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_పాఠ్యం_whitespace before = WHITESPACE_NONE;
	bool మొదటి = true;

	ఒకవేళ (html->layout == ఖాళీ) {
		తిరిగి NSERROR_INVALID;
	}

	తిరిగి selection_copy(html->layout,
			      &html->unit_len_ctx,
			      ప్రారంభం_idx,
			      చివర_idx,
			      selstr,
			      &before,
			      &మొదటి,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_చివర(నిర్మాణం content *c, గుర్తులేని *చివర_idx)
{
	html_content *html = (html_content *)c;
	గుర్తులేని మూలం_idx;

	ఒకవేళ (html->layout == ఖాళీ) {
		తిరిగి NSERROR_INVALID;
	}

	మూలం_idx = 0;

	*చివర_idx = selection_label_subtree(html->layout, మూలం_idx);

	తిరిగి NSERROR_OK;
}
