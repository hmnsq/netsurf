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

#تعریف_کن SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

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
	      بیعلامت شروع_idx,
	      بیعلامت انتها_idx,
	      بیعلامت *شروع_جابه_جایی,
	      بیعلامت *انتها_جابه_جایی)
{
	size_t box_درازی = box->درازی + SPACE_LEN(box);

	إذا (box_درازی > 0) {
		إذا ((box->byte_جابه_جایی >= شروع_idx) &&
		    (box->byte_جابه_جایی + box_درازی <= انتها_idx)) {

			/* fully enclosed */
			*شروع_جابه_جایی = 0;
			*انتها_جابه_جایی = box_درازی;
			أرجع true;
		} وإلا إذا ((box->byte_جابه_جایی + box_درازی > شروع_idx) &&
			   (box->byte_جابه_جایی < انتها_idx)) {
			/* partly enclosed */
			صحيح جابه_جایی = 0;
			صحيح len;

			إذا (box->byte_جابه_جایی < شروع_idx) {
				جابه_جایی = شروع_idx - box->byte_جابه_جایی;
			}

			len = box_درازی - جابه_جایی;

			إذا (box->byte_جابه_جایی + box_درازی > انتها_idx) {
				len = انتها_idx - (box->byte_جابه_جایی + جابه_جایی);
			}

			*شروع_جابه_جایی = جابه_جایی;
			*انتها_جابه_جایی = جابه_جایی + len;

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
		  بیعلامت شروع_idx,
		  بیعلامت انتها_idx,
		  بنية rdw_info *rdwi,
		  bool do_marker)
{
	بنية box *فرزند;
	nserror res;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->والد &&
	    box->والد->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->والد;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					شروع_idx,
					انتها_idx,
					rdwi,
					true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_جابه_جایی >= انتها_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	فرزند = box->children;

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_چپ ||
	       box->type == BOX_FLOAT_راست) &&
	      !box->متن)) {
		بیعلامت شروع_off;
		بیعلامت انتها_off;

		إذا (selected_part(box, شروع_idx, انتها_idx, &شروع_off, &انتها_off)) {
			صحيح عرض, ارتفاع;
			صحيح x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			عرض = box->padding[چپ_2] + box->عرض + box->padding[راست_2];
			ارتفاع = box->padding[بالاترین_2] + box->ارتفاع + box->padding[BOTTOM];

			إذا ((box->type == BOX_متن) &&
			    (box->space != 0)) {
				عرض += box->space;
			}

			إذا (rdwi->inited) {
				إذا (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				إذا (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				إذا (x + عرض > rdwi->r.x1) {
					rdwi->r.x1 = x + عرض;
				}
				إذا (y + ارتفاع > rdwi->r.y1) {
					rdwi->r.y1 = y + ارتفاع;
				}
			} وإلا {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + عرض;
				rdwi->r.y1 = y + ارتفاع;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (فرزند) {
		بنية box *بعدی = فرزند->بعدی;

		طالما (بعدی && بعدی->byte_جابه_جایی < شروع_idx) {
			فرزند = بعدی;
			بعدی = فرزند->بعدی;
		}

		طالما (فرزند) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *بعدی = فرزند->بعدی;

			res = coords_from_range(فرزند,
						شروع_idx,
						انتها_idx,
						rdwi,
						false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			فرزند = بعدی;
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
		   size_t درازی,
		   بنية box *box,
		   ثابت css_unit_ctx *unit_len_ctx,
		   بنية selection_string *handle,
		   ثابت حرف *whitespace_متن,
		   size_t whitespace_درازی)
{
	bool جمع_کن_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = عدم;

	/* add any whitespace which precedes the text from this box */
	إذا (whitespace_متن != عدم &&
	    whitespace_درازی > 0) {
		إذا (!selection_string_append(whitespace_متن,
					     whitespace_درازی,
					     false,
					     pstyle,
					     handle)) {
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (box != عدم) {
		/* HTML */
		جمع_کن_space = (box->space != 0);

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
	إذا (!selection_string_append(متن, درازی, جمع_کن_space, pstyle, handle)) {
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
	       بیعلامت شروع_idx,
	       بیعلامت انتها_idx,
	       بنية selection_string *selstr,
	       save_متن_whitespace *before,
	       bool *اولی,
	       bool do_marker)
{
	nserror res;
	بنية box *فرزند;
	ثابت حرف *whitespace_متن = "";
	size_t whitespace_درازی = 0;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->والد &&
	    box->والد->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->والد;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     شروع_idx,
				     انتها_idx,
				     selstr,
				     before,
				     اولی,
				     true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_جابه_جایی >= انتها_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	فرزند = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	إذا (before) {
		save_متن_solve_whitespace(box,
					   اولی,
					   before,
					   &whitespace_متن,
					   &whitespace_درازی);
	} وإلا {
		whitespace_متن = عدم;
	}

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_چپ ||
	       box->type == BOX_FLOAT_راست) &&
	      !box->متن)) {
		بیعلامت شروع_off;
		بیعلامت انتها_off;

		إذا (selected_part(box, شروع_idx, انتها_idx, &شروع_off, &انتها_off)) {
			res = selection_copy_box(box->متن + شروع_off,
						 min(box->درازی, انتها_off) - شروع_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_متن,
						 whitespace_درازی);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
			إذا (before) {
				*اولی = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (فرزند) {
		بنية box *بعدی = فرزند->بعدی;

		طالما (بعدی && بعدی->byte_جابه_جایی < شروع_idx) {
			فرزند = بعدی;
			بعدی = فرزند->بعدی;
		}

		طالما (فرزند) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *بعدی = فرزند->بعدی;

			res = selection_copy(فرزند,
					     unit_len_ctx,
					     شروع_idx,
					     انتها_idx,
					     selstr,
					     before,
					     اولی,
					     false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			فرزند = بعدی;
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
ساكن بیعلامت selection_label_subtree(بنية box *box, بیعلامت idx)
{
	بنية box *فرزند;

	أكد(box != عدم);

	فرزند = box->children;

	box->byte_جابه_جایی = idx;

	إذا (box->متن) {
		idx += box->درازی + SPACE_LEN(box);
	}

	طالما (فرزند) {
		إذا (فرزند->list_marker) {
			idx = selection_label_subtree(فرزند->list_marker, idx);
		}

		idx = selection_label_subtree(فرزند, idx);
		فرزند = فرزند->بعدی;
	}

	أرجع idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(بنية content *c,
			  بیعلامت شروع_idx,
			  بیعلامت انتها_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	بنية rdw_info rdw;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, شروع_idx, انتها_idx, &rdw, false);
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
			بیعلامت شروع_idx,
			بیعلامت انتها_idx,
			بنية selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_متن_whitespace before = WHITESPACE_NONE;
	bool اولی = true;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	أرجع selection_copy(html->layout,
			      &html->unit_len_ctx,
			      شروع_idx,
			      انتها_idx,
			      selstr,
			      &before,
			      &اولی,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_انتها(بنية content *c, بیعلامت *انتها_idx)
{
	html_content *html = (html_content *)c;
	بیعلامت ریشه_idx;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	ریشه_idx = 0;

	*انتها_idx = selection_label_subtree(html->layout, ریشه_idx);

	أرجع NSERROR_OK;
}
