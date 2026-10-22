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

#ضمن <stdlib.h>

#ضمن "utils/errors.h"
#ضمن "utils/utils.h"
#ضمن "netsurf/types.h"
#ضمن "netsurf/plot_style.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/save_text.h"

#ضمن "html/private.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/font.h"
#ضمن "html/textselection.h"

#تعریف_کرو SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

بنية rdw_info {
	bool inited;
	بنية rect r;
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
ساكن bool
selected_part(بنية box *box,
	      بنانشانی شروع_idx,
	      بنانشانی سرا_idx,
	      بنانشانی *شروع_وتھ,
	      بنانشانی *سرا_وتھ)
{
	size_t box_لمائی = box->لمائی + SPACE_LEN(box);

	إذا (box_لمائی > 0) {
		إذا ((box->byte_وتھ >= شروع_idx) &&
		    (box->byte_وتھ + box_لمائی <= سرا_idx)) {

			/* fully enclosed */
			*شروع_وتھ = 0;
			*سرا_وتھ = box_لمائی;
			أرجع true;
		} وإلا إذا ((box->byte_وتھ + box_لمائی > شروع_idx) &&
			   (box->byte_وتھ < سرا_idx)) {
			/* partly enclosed */
			صحيح وتھ = 0;
			صحيح len;

			إذا (box->byte_وتھ < شروع_idx) {
				وتھ = شروع_idx - box->byte_وتھ;
			}

			len = box_لمائی - وتھ;

			إذا (box->byte_وتھ + box_لمائی > سرا_idx) {
				len = سرا_idx - (box->byte_وتھ + وتھ);
			}

			*شروع_وتھ = وتھ;
			*سرا_وتھ = وتھ + len;

			أرجع true;
		}
	}
	أرجع false;
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
ساكن nserror
coords_from_range(بنية box *box,
		  بنانشانی شروع_idx,
		  بنانشانی سرا_idx,
		  بنية rdw_info *rdwi,
		  bool do_marker)
{
	بنية box *بچہ;
	nserror res;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->ماپا &&
	    box->ماپا->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ماپا;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					شروع_idx,
					سرا_idx,
					rdwi,
					true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_وتھ >= سرا_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	بچہ = box->children;

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_بایاں ||
	       box->type == BOX_FLOAT_دایاں) &&
	      !box->متن)) {
		بنانشانی شروع_off;
		بنانشانی سرا_off;

		إذا (selected_part(box, شروع_idx, سرا_idx, &شروع_off, &سرا_off)) {
			صحيح چوڑائی, اچائی;
			صحيح x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			چوڑائی = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2];
			اچائی = box->padding[چوٹی_2] + box->اچائی + box->padding[BOTTOM];

			إذا ((box->type == BOX_متن) &&
			    (box->space != 0)) {
				چوڑائی += box->space;
			}

			إذا (rdwi->inited) {
				إذا (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				إذا (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				إذا (x + چوڑائی > rdwi->r.x1) {
					rdwi->r.x1 = x + چوڑائی;
				}
				إذا (y + اچائی > rdwi->r.y1) {
					rdwi->r.y1 = y + اچائی;
				}
			} وإلا {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + چوڑائی;
				rdwi->r.y1 = y + اچائی;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (بچہ) {
		بنية box *اگلا = بچہ->اگلا;

		طالما (اگلا && اگلا->byte_وتھ < شروع_idx) {
			بچہ = اگلا;
			اگلا = بچہ->اگلا;
		}

		طالما (بچہ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *اگلا = بچہ->اگلا;

			res = coords_from_range(بچہ,
						شروع_idx,
						سرا_idx,
						rdwi,
						false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			بچہ = اگلا;
		}
	}

	أرجع NSERROR_OK;
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
ساكن nserror
selection_copy_box(ثابت حرف *متن,
		   size_t لمائی,
		   بنية box *box,
		   ثابت css_unit_ctx *unit_len_ctx,
		   بنية selection_string *handle,
		   ثابت حرف *whitespace_متن,
		   size_t whitespace_لمائی)
{
	bool جوڑو_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = عدم;

	/* add any whitespace which precedes the text from this box */
	إذا (whitespace_متن != عدم &&
	    whitespace_لمائی > 0) {
		إذا (!selection_string_append(whitespace_متن,
					     whitespace_لمائی,
					     false,
					     pstyle,
					     handle)) {
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (box != عدم) {
		/* HTML */
		جوڑو_space = (box->space != 0);

		إذا (box->style != عدم) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} وإلا {
			/* If there's no style, there must be no text */
			أكد(box->متن == عدم);
		}
	}

	/* add the text from this box */
	إذا (!selection_string_append(متن, لمائی, جوڑو_space, pstyle, handle)) {
		أرجع NSERROR_NOMEM;
	}

	أرجع NSERROR_OK;
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
ساكن nserror
selection_copy(بنية box *box,
	       ثابت css_unit_ctx *unit_len_ctx,
	       بنانشانی شروع_idx,
	       بنانشانی سرا_idx,
	       بنية selection_string *selstr,
	       save_متن_whitespace *before,
	       bool *پہلا,
	       bool do_marker)
{
	nserror res;
	بنية box *بچہ;
	ثابت حرف *whitespace_متن = "";
	size_t whitespace_لمائی = 0;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->ماپا &&
	    box->ماپا->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ماپا;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     شروع_idx,
				     سرا_idx,
				     selstr,
				     before,
				     پہلا,
				     true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_وتھ >= سرا_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	بچہ = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	إذا (before) {
		save_متن_solve_whitespace(box,
					   پہلا,
					   before,
					   &whitespace_متن,
					   &whitespace_لمائی);
	} وإلا {
		whitespace_متن = عدم;
	}

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_بایاں ||
	       box->type == BOX_FLOAT_دایاں) &&
	      !box->متن)) {
		بنانشانی شروع_off;
		بنانشانی سرا_off;

		إذا (selected_part(box, شروع_idx, سرا_idx, &شروع_off, &سرا_off)) {
			res = selection_copy_box(box->متن + شروع_off,
						 min(box->لمائی, سرا_off) - شروع_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_متن,
						 whitespace_لمائی);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
			إذا (before) {
				*پہلا = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (بچہ) {
		بنية box *اگلا = بچہ->اگلا;

		طالما (اگلا && اگلا->byte_وتھ < شروع_idx) {
			بچہ = اگلا;
			اگلا = بچہ->اگلا;
		}

		طالما (بچہ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *اگلا = بچہ->اگلا;

			res = selection_copy(بچہ,
					     unit_len_ctx,
					     شروع_idx,
					     سرا_idx,
					     selstr,
					     before,
					     پہلا,
					     false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			بچہ = اگلا;
		}
	}

	أرجع NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ساكن بنانشانی selection_label_subtree(بنية box *box, بنانشانی idx)
{
	بنية box *بچہ;

	أكد(box != عدم);

	بچہ = box->children;

	box->byte_وتھ = idx;

	إذا (box->متن) {
		idx += box->لمائی + SPACE_LEN(box);
	}

	طالما (بچہ) {
		إذا (بچہ->list_marker) {
			idx = selection_label_subtree(بچہ->list_marker, idx);
		}

		idx = selection_label_subtree(بچہ, idx);
		بچہ = بچہ->اگلا;
	}

	أرجع idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(بنية content *c,
			  بنانشانی شروع_idx,
			  بنانشانی سرا_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	بنية rdw_info rdw;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, شروع_idx, سرا_idx, &rdw, false);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	أرجع NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(بنية content *c,
			بنانشانی شروع_idx,
			بنانشانی سرا_idx,
			بنية selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_متن_whitespace before = WHITESPACE_NONE;
	bool پہلا = true;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	أرجع selection_copy(html->layout,
			      &html->unit_len_ctx,
			      شروع_idx,
			      سرا_idx,
			      selstr,
			      &before,
			      &پہلا,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_سرا(بنية content *c, بنانشانی *سرا_idx)
{
	html_content *html = (html_content *)c;
	بنانشانی جڑ_idx;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	جڑ_idx = 0;

	*سرا_idx = selection_label_subtree(html->layout, جڑ_idx);

	أرجع NSERROR_OK;
}
