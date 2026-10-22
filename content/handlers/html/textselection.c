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

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ساختار rdw_info {
	bool inited;
	ساختار rect r;
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
ایستا bool
selected_part(ساختار box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	اگر (box_length > 0) {
		اگر ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			بازگشت true;
		} وگرنه اگر ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			صحیح offset = 0;
			صحیح len;

			اگر (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			اگر (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			بازگشت true;
		}
	}
	بازگشت false;
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
ایستا nserror
coords_from_range(ساختار box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  ساختار rdw_info *rdwi,
		  bool do_marker)
{
	ساختار box *child;
	nserror res;

	أكد(box);

	/* If selection starts inside marker */
	اگر (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	اگر (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	اگر (box->byte_offset >= end_idx) {
		بازگشت NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	اگر ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_چپ ||
	       box->type == BOX_FLOAT_راست) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		اگر (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			صحیح width, height;
			صحیح x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[چپ_2] + box->width + box->padding[راست_2];
			height = box->padding[بالاترین_2] + box->height + box->padding[BOTTOM];

			اگر ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			اگر (rdwi->inited) {
				اگر (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				اگر (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				اگر (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				اگر (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} وگرنه {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + width;
				rdwi->r.y1 = y + height;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	اگر (child) {
		ساختار box *next = child->next;

		وقتی (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		وقتی (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ساختار box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}

			child = next;
		}
	}

	بازگشت NSERROR_OK;
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
ایستا nserror
selection_copy_box(ثابت نویسه *text,
		   size_t length,
		   ساختار box *box,
		   ثابت css_unit_ctx *unit_len_ctx,
		   ساختار selection_string *handle,
		   ثابت نویسه *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = پوچ;

	/* add any whitespace which precedes the text from this box */
	اگر (whitespace_text != پوچ &&
	    whitespace_length > 0) {
		اگر (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			بازگشت NSERROR_NOMEM;
		}
	}

	اگر (box != پوچ) {
		/* HTML */
		add_space = (box->space != 0);

		اگر (box->style != پوچ) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} وگرنه {
			/* If there's no style, there must be no text */
			أكد(box->text == پوچ);
		}
	}

	/* add the text from this box */
	اگر (!selection_string_append(text, length, add_space, pstyle, handle)) {
		بازگشت NSERROR_NOMEM;
	}

	بازگشت NSERROR_OK;
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
ایستا nserror
selection_copy(ساختار box *box,
	       ثابت css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       ساختار selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	ساختار box *child;
	ثابت نویسه *whitespace_text = "";
	size_t whitespace_length = 0;

	أكد(box);

	/* If selection starts inside marker */
	اگر (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	اگر (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     start_idx,
				     end_idx,
				     selstr,
				     before,
				     first,
				     true);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	اگر (box->byte_offset >= end_idx) {
		بازگشت NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	اگر (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} وگرنه {
		whitespace_text = پوچ;
	}

	اگر ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_چپ ||
	       box->type == BOX_FLOAT_راست) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		اگر (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}
			اگر (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	اگر (child) {
		ساختار box *next = child->next;

		وقتی (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		وقتی (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ساختار box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}

			child = next;
		}
	}

	بازگشت NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ایستا unsigned selection_label_subtree(ساختار box *box, unsigned idx)
{
	ساختار box *child;

	أكد(box != پوچ);

	child = box->children;

	box->byte_offset = idx;

	اگر (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	وقتی (child) {
		اگر (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	بازگشت idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ساختار content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ساختار rdw_info rdw;

	اگر (html->layout == پوچ) {
		بازگشت NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	اگر (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	بازگشت NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ساختار content *c,
			unsigned start_idx,
			unsigned end_idx,
			ساختار selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	اگر (html->layout == پوچ) {
		بازگشت NSERROR_INVALID;
	}

	بازگشت selection_copy(html->layout,
			      &html->unit_len_ctx,
			      start_idx,
			      end_idx,
			      selstr,
			      &before,
			      &first,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_end(ساختار content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned ریشه_idx;

	اگر (html->layout == پوچ) {
		بازگشت NSERROR_INVALID;
	}

	ریشه_idx = 0;

	*end_idx = selection_label_subtree(html->layout, ریشه_idx);

	بازگشت NSERROR_OK;
}
