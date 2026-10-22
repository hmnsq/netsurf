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

#include <stdlib.h>

#include "utils/errors.h"
#include "utils/utils.h"
#include "netsurf/types.h"
#include "netsurf/plot_style.h"
#include "desktop/selection.h"
#include "desktop/save_text.h"

#include "html/private.h"
#include "html/box.h"
#include "html/box_inspect.h"
#include "html/font.h"
#include "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

struct rdw_info {
	bool inited;
	struct rect r;
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
static bool
selected_part(struct box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	如果 (box_length > 0) {
		如果 ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			返回 true;
		} 否则 如果 ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			整数 offset = 0;
			整数 len;

			如果 (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			如果 (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			返回 true;
		}
	}
	返回 false;
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
static nserror
coords_from_range(struct box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  struct rdw_info *rdwi,
		  bool do_marker)
{
	struct box *child;
	nserror res;

	assert(box);

	/* If selection starts inside marker */
	如果 (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	如果 (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		如果 (res != NSERROR_OK) {
			返回 res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	如果 (box->byte_offset >= end_idx) {
		返回 NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	如果 ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_LEFT ||
	       box->type == BOX_FLOAT_RIGHT) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		如果 (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			整数 width, height;
			整数 x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[LEFT] + box->width + box->padding[RIGHT];
			height = box->padding[TOP] + box->height + box->padding[BOTTOM];

			如果 ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			如果 (rdwi->inited) {
				如果 (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				如果 (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				如果 (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				如果 (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} 否则 {
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

	如果 (child) {
		struct box *next = child->next;

		当 (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		当 (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			struct box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			如果 (res != NSERROR_OK) {
				返回 res;
			}

			child = next;
		}
	}

	返回 NSERROR_OK;
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
static nserror
selection_copy_box(const char *text,
		   size_t length,
		   struct box *box,
		   const css_unit_ctx *unit_len_ctx,
		   struct selection_string *handle,
		   const char *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = NULL;

	/* add any whitespace which precedes the text from this box */
	如果 (whitespace_text != NULL &&
	    whitespace_length > 0) {
		如果 (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			返回 NSERROR_NOMEM;
		}
	}

	如果 (box != NULL) {
		/* HTML */
		add_space = (box->space != 0);

		如果 (box->style != NULL) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} 否则 {
			/* If there's no style, there must be no text */
			assert(box->text == NULL);
		}
	}

	/* add the text from this box */
	如果 (!selection_string_append(text, length, add_space, pstyle, handle)) {
		返回 NSERROR_NOMEM;
	}

	返回 NSERROR_OK;
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
static nserror
selection_copy(struct box *box,
	       const css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       struct selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	struct box *child;
	const char *whitespace_text = "";
	size_t whitespace_length = 0;

	assert(box);

	/* If selection starts inside marker */
	如果 (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	如果 (box->list_marker) {
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
		如果 (res != NSERROR_OK) {
			返回 res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	如果 (box->byte_offset >= end_idx) {
		返回 NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	如果 (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} 否则 {
		whitespace_text = NULL;
	}

	如果 ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_LEFT ||
	       box->type == BOX_FLOAT_RIGHT) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		如果 (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			如果 (res != NSERROR_OK) {
				返回 res;
			}
			如果 (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	如果 (child) {
		struct box *next = child->next;

		当 (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		当 (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			struct box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			如果 (res != NSERROR_OK) {
				返回 res;
			}

			child = next;
		}
	}

	返回 NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
static unsigned selection_label_subtree(struct box *box, unsigned idx)
{
	struct box *child;

	assert(box != NULL);

	child = box->children;

	box->byte_offset = idx;

	如果 (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	当 (child) {
		如果 (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	返回 idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(struct content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	struct rdw_info rdw;

	如果 (html->layout == NULL) {
		返回 NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	如果 (res != NSERROR_OK) {
		返回 res;
	}

	如果 (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	返回 NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(struct content *c,
			unsigned start_idx,
			unsigned end_idx,
			struct selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	如果 (html->layout == NULL) {
		返回 NSERROR_INVALID;
	}

	返回 selection_copy(html->layout,
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
html_textselection_get_end(struct content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned root_idx;

	如果 (html->layout == NULL) {
		返回 NSERROR_INVALID;
	}

	root_idx = 0;

	*end_idx = selection_label_subtree(html->layout, root_idx);

	返回 NSERROR_OK;
}
