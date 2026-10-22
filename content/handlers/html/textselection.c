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

#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "netsurf/plot_style.h"
#অন্তর্ভাব "desktop/selection.h"
#অন্তর্ভাব "desktop/save_text.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

সংরচনা rdw_info {
	bool inited;
	সংরচনা rect r;
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
স্থির bool
selected_part(সংরচনা box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	যদি (box_length > 0) {
		যদি ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			প্রত্যাগম true;
		} অন্যথা যদি ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			পূর্ণাঙ্ক offset = 0;
			পূর্ণাঙ্ক len;

			যদি (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			যদি (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			প্রত্যাগম true;
		}
	}
	প্রত্যাগম false;
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
স্থির nserror
coords_from_range(সংরচনা box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  সংরচনা rdw_info *rdwi,
		  bool do_marker)
{
	সংরচনা box *child;
	nserror res;

	প্রতিজ্ঞা(box);

	/* If selection starts inside marker */
	যদি (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	যদি (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		যদি (res != NSERROR_OK) {
			প্রত্যাগম res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	যদি (box->byte_offset >= end_idx) {
		প্রত্যাগম NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	যদি ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_বাম ||
	       box->type == BOX_FLOAT_ডান) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		যদি (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			পূর্ণাঙ্ক width, height;
			পূর্ণাঙ্ক x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[বাম_2] + box->width + box->padding[ডান_2];
			height = box->padding[শীর্ষ_2] + box->height + box->padding[BOTTOM];

			যদি ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			যদি (rdwi->inited) {
				যদি (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				যদি (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				যদি (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				যদি (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} অন্যথা {
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

	যদি (child) {
		সংরচনা box *next = child->next;

		যাবত্ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		যাবত্ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			সংরচনা box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			যদি (res != NSERROR_OK) {
				প্রত্যাগম res;
			}

			child = next;
		}
	}

	প্রত্যাগম NSERROR_OK;
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
স্থির nserror
selection_copy_box(ধ্রুব বর্ণ *text,
		   size_t length,
		   সংরচনা box *box,
		   ধ্রুব css_unit_ctx *unit_len_ctx,
		   সংরচনা selection_string *handle,
		   ধ্রুব বর্ণ *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = রিক্ত;

	/* add any whitespace which precedes the text from this box */
	যদি (whitespace_text != রিক্ত &&
	    whitespace_length > 0) {
		যদি (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			প্রত্যাগম NSERROR_NOMEM;
		}
	}

	যদি (box != রিক্ত) {
		/* HTML */
		add_space = (box->space != 0);

		যদি (box->style != রিক্ত) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} অন্যথা {
			/* If there's no style, there must be no text */
			প্রতিজ্ঞা(box->text == রিক্ত);
		}
	}

	/* add the text from this box */
	যদি (!selection_string_append(text, length, add_space, pstyle, handle)) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	প্রত্যাগম NSERROR_OK;
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
স্থির nserror
selection_copy(সংরচনা box *box,
	       ধ্রুব css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       সংরচনা selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	সংরচনা box *child;
	ধ্রুব বর্ণ *whitespace_text = "";
	size_t whitespace_length = 0;

	প্রতিজ্ঞা(box);

	/* If selection starts inside marker */
	যদি (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	যদি (box->list_marker) {
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
		যদি (res != NSERROR_OK) {
			প্রত্যাগম res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	যদি (box->byte_offset >= end_idx) {
		প্রত্যাগম NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	যদি (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} অন্যথা {
		whitespace_text = রিক্ত;
	}

	যদি ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_বাম ||
	       box->type == BOX_FLOAT_ডান) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		যদি (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			যদি (res != NSERROR_OK) {
				প্রত্যাগম res;
			}
			যদি (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	যদি (child) {
		সংরচনা box *next = child->next;

		যাবত্ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		যাবত্ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			সংরচনা box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			যদি (res != NSERROR_OK) {
				প্রত্যাগম res;
			}

			child = next;
		}
	}

	প্রত্যাগম NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
স্থির unsigned selection_label_subtree(সংরচনা box *box, unsigned idx)
{
	সংরচনা box *child;

	প্রতিজ্ঞা(box != রিক্ত);

	child = box->children;

	box->byte_offset = idx;

	যদি (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	যাবত্ (child) {
		যদি (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	প্রত্যাগম idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(সংরচনা content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	সংরচনা rdw_info rdw;

	যদি (html->layout == রিক্ত) {
		প্রত্যাগম NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	যদি (res != NSERROR_OK) {
		প্রত্যাগম res;
	}

	যদি (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(সংরচনা content *c,
			unsigned start_idx,
			unsigned end_idx,
			সংরচনা selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	যদি (html->layout == রিক্ত) {
		প্রত্যাগম NSERROR_INVALID;
	}

	প্রত্যাগম selection_copy(html->layout,
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
html_textselection_get_end(সংরচনা content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned মূল_idx;

	যদি (html->layout == রিক্ত) {
		প্রত্যাগম NSERROR_INVALID;
	}

	মূল_idx = 0;

	*end_idx = selection_label_subtree(html->layout, মূল_idx);

	প্রত্যাগম NSERROR_OK;
}
