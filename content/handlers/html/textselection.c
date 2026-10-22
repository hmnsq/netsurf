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

#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "netsurf/plot_style.h"
#அந்தர்பாவ "desktop/selection.h"
#அந்தர்பாவ "desktop/save_text.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

கட்டமைப்பு rdw_info {
	bool inited;
	கட்டமைப்பு rect r;
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
நிலையான bool
selected_part(கட்டமைப்பு box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	எனில் (box_length > 0) {
		எனில் ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			திரும்பு true;
		} இல்லையெனில் எனில் ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			முழு offset = 0;
			முழு len;

			எனில் (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			எனில் (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			திரும்பு true;
		}
	}
	திரும்பு false;
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
நிலையான nserror
coords_from_range(கட்டமைப்பு box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  கட்டமைப்பு rdw_info *rdwi,
		  bool do_marker)
{
	கட்டமைப்பு box *child;
	nserror res;

	ப்ரதிஜ்ஞா(box);

	/* If selection starts inside marker */
	எனில் (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	எனில் (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	எனில் (box->byte_offset >= end_idx) {
		திரும்பு NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	எனில் ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_இடது ||
	       box->type == BOX_FLOAT_வலது) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		எனில் (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			முழு width, height;
			முழு x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[இடது_2] + box->width + box->padding[வலது_2];
			height = box->padding[உச்சி_2] + box->height + box->padding[BOTTOM];

			எனில் ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			எனில் (rdwi->inited) {
				எனில் (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				எனில் (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				எனில் (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				எனில் (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} இல்லையெனில் {
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

	எனில் (child) {
		கட்டமைப்பு box *next = child->next;

		போது (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		போது (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			கட்டமைப்பு box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}

			child = next;
		}
	}

	திரும்பு NSERROR_OK;
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
நிலையான nserror
selection_copy_box(மாறிலி எழுத்து *text,
		   size_t length,
		   கட்டமைப்பு box *box,
		   மாறிலி css_unit_ctx *unit_len_ctx,
		   கட்டமைப்பு selection_string *handle,
		   மாறிலி எழுத்து *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = வெற்று;

	/* add any whitespace which precedes the text from this box */
	எனில் (whitespace_text != வெற்று &&
	    whitespace_length > 0) {
		எனில் (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			திரும்பு NSERROR_NOMEM;
		}
	}

	எனில் (box != வெற்று) {
		/* HTML */
		add_space = (box->space != 0);

		எனில் (box->style != வெற்று) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} இல்லையெனில் {
			/* If there's no style, there must be no text */
			ப்ரதிஜ்ஞா(box->text == வெற்று);
		}
	}

	/* add the text from this box */
	எனில் (!selection_string_append(text, length, add_space, pstyle, handle)) {
		திரும்பு NSERROR_NOMEM;
	}

	திரும்பு NSERROR_OK;
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
நிலையான nserror
selection_copy(கட்டமைப்பு box *box,
	       மாறிலி css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       கட்டமைப்பு selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	கட்டமைப்பு box *child;
	மாறிலி எழுத்து *whitespace_text = "";
	size_t whitespace_length = 0;

	ப்ரதிஜ்ஞா(box);

	/* If selection starts inside marker */
	எனில் (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	எனில் (box->list_marker) {
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
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	எனில் (box->byte_offset >= end_idx) {
		திரும்பு NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	எனில் (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} இல்லையெனில் {
		whitespace_text = வெற்று;
	}

	எனில் ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_இடது ||
	       box->type == BOX_FLOAT_வலது) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		எனில் (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}
			எனில் (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	எனில் (child) {
		கட்டமைப்பு box *next = child->next;

		போது (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		போது (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			கட்டமைப்பு box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}

			child = next;
		}
	}

	திரும்பு NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
நிலையான unsigned selection_label_subtree(கட்டமைப்பு box *box, unsigned idx)
{
	கட்டமைப்பு box *child;

	ப்ரதிஜ்ஞா(box != வெற்று);

	child = box->children;

	box->byte_offset = idx;

	எனில் (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	போது (child) {
		எனில் (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	திரும்பு idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(கட்டமைப்பு content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	கட்டமைப்பு rdw_info rdw;

	எனில் (html->layout == வெற்று) {
		திரும்பு NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(கட்டமைப்பு content *c,
			unsigned start_idx,
			unsigned end_idx,
			கட்டமைப்பு selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	எனில் (html->layout == வெற்று) {
		திரும்பு NSERROR_INVALID;
	}

	திரும்பு selection_copy(html->layout,
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
html_textselection_get_end(கட்டமைப்பு content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned வேர்_idx;

	எனில் (html->layout == வெற்று) {
		திரும்பு NSERROR_INVALID;
	}

	வேர்_idx = 0;

	*end_idx = selection_label_subtree(html->layout, வேர்_idx);

	திரும்பு NSERROR_OK;
}
