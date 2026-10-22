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

#شامل <stdlib.h>

#شامل "utils/errors.h"
#شامل "utils/utils.h"
#شامل "netsurf/types.h"
#شامل "netsurf/plot_style.h"
#شامل "desktop/selection.h"
#شامل "desktop/save_text.h"

#شامل "html/private.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/textselection.h"

#تعریف_کرو SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ساخت rdw_info {
	bool inited;
	ساخت rect r;
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
ساکن bool
selected_part(ساخت box *box,
	      غیرعلامتی آغاز_idx,
	      غیرعلامتی انتہا_idx,
	      غیرعلامتی *آغاز_انحراف,
	      غیرعلامتی *انتہا_انحراف)
{
	size_t box_لمبائی = box->لمبائی + SPACE_LEN(box);

	اگر (box_لمبائی > 0) {
		اگر ((box->byte_انحراف >= آغاز_idx) &&
		    (box->byte_انحراف + box_لمبائی <= انتہا_idx)) {

			/* fully enclosed */
			*آغاز_انحراف = 0;
			*انتہا_انحراف = box_لمبائی;
			واپس true;
		} ورنہ اگر ((box->byte_انحراف + box_لمبائی > آغاز_idx) &&
			   (box->byte_انحراف < انتہا_idx)) {
			/* partly enclosed */
			صحیح انحراف = 0;
			صحیح len;

			اگر (box->byte_انحراف < آغاز_idx) {
				انحراف = آغاز_idx - box->byte_انحراف;
			}

			len = box_لمبائی - انحراف;

			اگر (box->byte_انحراف + box_لمبائی > انتہا_idx) {
				len = انتہا_idx - (box->byte_انحراف + انحراف);
			}

			*آغاز_انحراف = انحراف;
			*انتہا_انحراف = انحراف + len;

			واپس true;
		}
	}
	واپس false;
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
ساکن nserror
coords_from_range(ساخت box *box,
		  غیرعلامتی آغاز_idx,
		  غیرعلامتی انتہا_idx,
		  ساخت rdw_info *rdwi,
		  bool do_marker)
{
	ساخت box *بچہ;
	nserror res;

	أكد(box);

	/* If selection starts inside marker */
	اگر (box->والد &&
	    box->والد->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->والد;
	}

	/* If box has a list marker */
	اگر (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					آغاز_idx,
					انتہا_idx,
					rdwi,
					true);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	اگر (box->byte_انحراف >= انتہا_idx) {
		واپس NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	بچہ = box->children;

	اگر ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_بایاں ||
	       box->type == BOX_FLOAT_دایاں) &&
	      !box->متن)) {
		غیرعلامتی آغاز_off;
		غیرعلامتی انتہا_off;

		اگر (selected_part(box, آغاز_idx, انتہا_idx, &آغاز_off, &انتہا_off)) {
			صحیح چوڑائی, اونچائی;
			صحیح x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			چوڑائی = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2];
			اونچائی = box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM];

			اگر ((box->type == BOX_متن) &&
			    (box->space != 0)) {
				چوڑائی += box->space;
			}

			اگر (rdwi->inited) {
				اگر (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				اگر (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				اگر (x + چوڑائی > rdwi->r.x1) {
					rdwi->r.x1 = x + چوڑائی;
				}
				اگر (y + اونچائی > rdwi->r.y1) {
					rdwi->r.y1 = y + اونچائی;
				}
			} ورنہ {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + چوڑائی;
				rdwi->r.y1 = y + اونچائی;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	اگر (بچہ) {
		ساخت box *اگلا = بچہ->اگلا;

		جب (اگلا && اگلا->byte_انحراف < آغاز_idx) {
			بچہ = اگلا;
			اگلا = بچہ->اگلا;
		}

		جب (بچہ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ساخت box *اگلا = بچہ->اگلا;

			res = coords_from_range(بچہ,
						آغاز_idx,
						انتہا_idx,
						rdwi,
						false);
			اگر (res != NSERROR_OK) {
				واپس res;
			}

			بچہ = اگلا;
		}
	}

	واپس NSERROR_OK;
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
ساکن nserror
selection_copy_box(ثابت حرف *متن,
		   size_t لمبائی,
		   ساخت box *box,
		   ثابت css_unit_ctx *unit_len_ctx,
		   ساخت selection_string *handle,
		   ثابت حرف *whitespace_متن,
		   size_t whitespace_لمبائی)
{
	bool جوڑو_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = تہی;

	/* add any whitespace which precedes the text from this box */
	اگر (whitespace_متن != تہی &&
	    whitespace_لمبائی > 0) {
		اگر (!selection_string_append(whitespace_متن,
					     whitespace_لمبائی,
					     false,
					     pstyle,
					     handle)) {
			واپس NSERROR_NOMEM;
		}
	}

	اگر (box != تہی) {
		/* HTML */
		جوڑو_space = (box->space != 0);

		اگر (box->style != تہی) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} ورنہ {
			/* If there's no style, there must be no text */
			أكد(box->متن == تہی);
		}
	}

	/* add the text from this box */
	اگر (!selection_string_append(متن, لمبائی, جوڑو_space, pstyle, handle)) {
		واپس NSERROR_NOMEM;
	}

	واپس NSERROR_OK;
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
ساکن nserror
selection_copy(ساخت box *box,
	       ثابت css_unit_ctx *unit_len_ctx,
	       غیرعلامتی آغاز_idx,
	       غیرعلامتی انتہا_idx,
	       ساخت selection_string *selstr,
	       save_متن_whitespace *before,
	       bool *پہلا,
	       bool do_marker)
{
	nserror res;
	ساخت box *بچہ;
	ثابت حرف *whitespace_متن = "";
	size_t whitespace_لمبائی = 0;

	أكد(box);

	/* If selection starts inside marker */
	اگر (box->والد &&
	    box->والد->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->والد;
	}

	/* If box has a list marker */
	اگر (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     آغاز_idx,
				     انتہا_idx,
				     selstr,
				     before,
				     پہلا,
				     true);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	اگر (box->byte_انحراف >= انتہا_idx) {
		واپس NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	بچہ = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	اگر (before) {
		save_متن_solve_whitespace(box,
					   پہلا,
					   before,
					   &whitespace_متن,
					   &whitespace_لمبائی);
	} ورنہ {
		whitespace_متن = تہی;
	}

	اگر ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_بایاں ||
	       box->type == BOX_FLOAT_دایاں) &&
	      !box->متن)) {
		غیرعلامتی آغاز_off;
		غیرعلامتی انتہا_off;

		اگر (selected_part(box, آغاز_idx, انتہا_idx, &آغاز_off, &انتہا_off)) {
			res = selection_copy_box(box->متن + آغاز_off,
						 min(box->لمبائی, انتہا_off) - آغاز_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_متن,
						 whitespace_لمبائی);
			اگر (res != NSERROR_OK) {
				واپس res;
			}
			اگر (before) {
				*پہلا = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	اگر (بچہ) {
		ساخت box *اگلا = بچہ->اگلا;

		جب (اگلا && اگلا->byte_انحراف < آغاز_idx) {
			بچہ = اگلا;
			اگلا = بچہ->اگلا;
		}

		جب (بچہ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ساخت box *اگلا = بچہ->اگلا;

			res = selection_copy(بچہ,
					     unit_len_ctx,
					     آغاز_idx,
					     انتہا_idx,
					     selstr,
					     before,
					     پہلا,
					     false);
			اگر (res != NSERROR_OK) {
				واپس res;
			}

			بچہ = اگلا;
		}
	}

	واپس NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ساکن غیرعلامتی selection_label_subtree(ساخت box *box, غیرعلامتی idx)
{
	ساخت box *بچہ;

	أكد(box != تہی);

	بچہ = box->children;

	box->byte_انحراف = idx;

	اگر (box->متن) {
		idx += box->لمبائی + SPACE_LEN(box);
	}

	جب (بچہ) {
		اگر (بچہ->list_marker) {
			idx = selection_label_subtree(بچہ->list_marker, idx);
		}

		idx = selection_label_subtree(بچہ, idx);
		بچہ = بچہ->اگلا;
	}

	واپس idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ساخت content *c,
			  غیرعلامتی آغاز_idx,
			  غیرعلامتی انتہا_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ساخت rdw_info rdw;

	اگر (html->layout == تہی) {
		واپس NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, آغاز_idx, انتہا_idx, &rdw, false);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	واپس NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ساخت content *c,
			غیرعلامتی آغاز_idx,
			غیرعلامتی انتہا_idx,
			ساخت selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_متن_whitespace before = WHITESPACE_NONE;
	bool پہلا = true;

	اگر (html->layout == تہی) {
		واپس NSERROR_INVALID;
	}

	واپس selection_copy(html->layout,
			      &html->unit_len_ctx,
			      آغاز_idx,
			      انتہا_idx,
			      selstr,
			      &before,
			      &پہلا,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_انتہا(ساخت content *c, غیرعلامتی *انتہا_idx)
{
	html_content *html = (html_content *)c;
	غیرعلامتی جڑ_idx;

	اگر (html->layout == تہی) {
		واپس NSERROR_INVALID;
	}

	جڑ_idx = 0;

	*انتہا_idx = selection_label_subtree(html->layout, جڑ_idx);

	واپس NSERROR_OK;
}
