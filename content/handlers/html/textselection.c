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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plot_style.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"
#ଅନ୍ତର୍ଭାଵ "desktop/save_text.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ସଂରଚନା rdw_info {
	bool inited;
	ସଂରଚନା rect r;
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
ସ୍ଥିର bool
selected_part(ସଂରଚନା box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	ଯଦି (box_length > 0) {
		ଯଦି ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			ଫେରନ୍ତୁ true;
		} ଅନ୍ୟଥା ଯଦି ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 0;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

			ଯଦି (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			ଯଦି (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			ଫେରନ୍ତୁ true;
		}
	}
	ଫେରନ୍ତୁ false;
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
ସ୍ଥିର nserror
coords_from_range(ସଂରଚନା box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  ସଂରଚନା rdw_info *rdwi,
		  bool do_marker)
{
	ସଂରଚନା box *child;
	nserror res;

	ପ୍ରତିଜ୍ଞା(box);

	/* If selection starts inside marker */
	ଯଦି (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	ଯଦି (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ଯଦି (box->byte_offset >= end_idx) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	ଯଦି ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ବାମ ||
	       box->type == BOX_FLOAT_ଡାହାଣ) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		ଯଦି (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2];
			height = box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM];

			ଯଦି ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			ଯଦି (rdwi->inited) {
				ଯଦି (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				ଯଦି (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				ଯଦି (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				ଯଦି (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} ଅନ୍ୟଥା {
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

	ଯଦି (child) {
		ସଂରଚନା box *next = child->next;

		ଯେତେବେଳେ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		ଯେତେବେଳେ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ସଂରଚନା box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}

			child = next;
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
selection_copy_box(ଅଚଳ ଅକ୍ଷର *text,
		   size_t length,
		   ସଂରଚନା box *box,
		   ଅଚଳ css_unit_ctx *unit_len_ctx,
		   ସଂରଚନା selection_string *handle,
		   ଅଚଳ ଅକ୍ଷର *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = ଖାଲି;

	/* add any whitespace which precedes the text from this box */
	ଯଦି (whitespace_text != ଖାଲି &&
	    whitespace_length > 0) {
		ଯଦି (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ଯଦି (box != ଖାଲି) {
		/* HTML */
		add_space = (box->space != 0);

		ଯଦି (box->style != ଖାଲି) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} ଅନ୍ୟଥା {
			/* If there's no style, there must be no text */
			ପ୍ରତିଜ୍ଞା(box->text == ଖାଲି);
		}
	}

	/* add the text from this box */
	ଯଦି (!selection_string_append(text, length, add_space, pstyle, handle)) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
selection_copy(ସଂରଚନା box *box,
	       ଅଚଳ css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       ସଂରଚନା selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	ସଂରଚନା box *child;
	ଅଚଳ ଅକ୍ଷର *whitespace_text = "";
	size_t whitespace_length = 0;

	ପ୍ରତିଜ୍ଞା(box);

	/* If selection starts inside marker */
	ଯଦି (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	ଯଦି (box->list_marker) {
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
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ଯଦି (box->byte_offset >= end_idx) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	ଯଦି (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} ଅନ୍ୟଥା {
		whitespace_text = ଖାଲି;
	}

	ଯଦି ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ବାମ ||
	       box->type == BOX_FLOAT_ଡାହାଣ) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		ଯଦି (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
			ଯଦି (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ଯଦି (child) {
		ସଂରଚନା box *next = child->next;

		ଯେତେବେଳେ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		ଯେତେବେଳେ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ସଂରଚନା box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}

			child = next;
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ସ୍ଥିର unsigned selection_label_subtree(ସଂରଚନା box *box, unsigned idx)
{
	ସଂରଚନା box *child;

	ପ୍ରତିଜ୍ଞା(box != ଖାଲି);

	child = box->children;

	box->byte_offset = idx;

	ଯଦି (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	ଯେତେବେଳେ (child) {
		ଯଦି (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	ଫେରନ୍ତୁ idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ସଂରଚନା content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ସଂରଚନା rdw_info rdw;

	ଯଦି (html->layout == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ସଂରଚନା content *c,
			unsigned start_idx,
			unsigned end_idx,
			ସଂରଚନା selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	ଯଦି (html->layout == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	ଫେରନ୍ତୁ selection_copy(html->layout,
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
html_textselection_get_end(ସଂରଚନା content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned ମୂଳ_idx;

	ଯଦି (html->layout == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	ମୂଳ_idx = 0;

	*end_idx = selection_label_subtree(html->layout, ମୂଳ_idx);

	ଫେରନ୍ତୁ NSERROR_OK;
}
