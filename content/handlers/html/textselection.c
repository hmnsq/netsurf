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

#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "netsurf/plot_style.h"
#അന്തര്ഭാവ "desktop/selection.h"
#അന്തര്ഭാവ "desktop/save_text.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ഘടന rdw_info {
	bool inited;
	ഘടന rect r;
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
സ്ഥിതം bool
selected_part(ഘടന box *box,
	      unsigned start_idx,
	      unsigned end_idx,
	      unsigned *start_offset,
	      unsigned *end_offset)
{
	size_t box_length = box->length + SPACE_LEN(box);

	എങ്കിൽ (box_length > 0) {
		എങ്കിൽ ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			തിരികെ true;
		} അല്ലെങ്കിൽ എങ്കിൽ ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			പൂർണ്ണസംഖ്യ offset = 0;
			പൂർണ്ണസംഖ്യ len;

			എങ്കിൽ (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			എങ്കിൽ (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			തിരികെ true;
		}
	}
	തിരികെ false;
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
സ്ഥിതം nserror
coords_from_range(ഘടന box *box,
		  unsigned start_idx,
		  unsigned end_idx,
		  ഘടന rdw_info *rdwi,
		  bool do_marker)
{
	ഘടന box *child;
	nserror res;

	പ്രതിജ്ഞാ(box);

	/* If selection starts inside marker */
	എങ്കിൽ (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	എങ്കിൽ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	എങ്കിൽ (box->byte_offset >= end_idx) {
		തിരികെ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	എങ്കിൽ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ഇടത് ||
	       box->type == BOX_FLOAT_വലത്) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		എങ്കിൽ (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			പൂർണ്ണസംഖ്യ width, height;
			പൂർണ്ണസംഖ്യ x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2];
			height = box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM];

			എങ്കിൽ ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			എങ്കിൽ (rdwi->inited) {
				എങ്കിൽ (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				എങ്കിൽ (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				എങ്കിൽ (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				എങ്കിൽ (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} അല്ലെങ്കിൽ {
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

	എങ്കിൽ (child) {
		ഘടന box *next = child->next;

		ആകുമ്പോൾ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		ആകുമ്പോൾ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ഘടന box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}

			child = next;
		}
	}

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror
selection_copy_box(സ്ഥിരം അക്ഷരം *text,
		   size_t length,
		   ഘടന box *box,
		   സ്ഥിരം css_unit_ctx *unit_len_ctx,
		   ഘടന selection_string *handle,
		   സ്ഥിരം അക്ഷരം *whitespace_text,
		   size_t whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = ശൂന്യ;

	/* add any whitespace which precedes the text from this box */
	എങ്കിൽ (whitespace_text != ശൂന്യ &&
	    whitespace_length > 0) {
		എങ്കിൽ (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			തിരികെ NSERROR_NOMEM;
		}
	}

	എങ്കിൽ (box != ശൂന്യ) {
		/* HTML */
		add_space = (box->space != 0);

		എങ്കിൽ (box->style != ശൂന്യ) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} അല്ലെങ്കിൽ {
			/* If there's no style, there must be no text */
			പ്രതിജ്ഞാ(box->text == ശൂന്യ);
		}
	}

	/* add the text from this box */
	എങ്കിൽ (!selection_string_append(text, length, add_space, pstyle, handle)) {
		തിരികെ NSERROR_NOMEM;
	}

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror
selection_copy(ഘടന box *box,
	       സ്ഥിരം css_unit_ctx *unit_len_ctx,
	       unsigned start_idx,
	       unsigned end_idx,
	       ഘടന selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	ഘടന box *child;
	സ്ഥിരം അക്ഷരം *whitespace_text = "";
	size_t whitespace_length = 0;

	പ്രതിജ്ഞാ(box);

	/* If selection starts inside marker */
	എങ്കിൽ (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	എങ്കിൽ (box->list_marker) {
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
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	എങ്കിൽ (box->byte_offset >= end_idx) {
		തിരികെ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	എങ്കിൽ (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} അല്ലെങ്കിൽ {
		whitespace_text = ശൂന്യ;
	}

	എങ്കിൽ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ഇടത് ||
	       box->type == BOX_FLOAT_വലത്) &&
	      !box->text)) {
		unsigned start_off;
		unsigned end_off;

		എങ്കിൽ (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}
			എങ്കിൽ (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	എങ്കിൽ (child) {
		ഘടന box *next = child->next;

		ആകുമ്പോൾ (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		ആകുമ്പോൾ (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ഘടന box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}

			child = next;
		}
	}

	തിരികെ NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
സ്ഥിതം unsigned selection_label_subtree(ഘടന box *box, unsigned idx)
{
	ഘടന box *child;

	പ്രതിജ്ഞാ(box != ശൂന്യ);

	child = box->children;

	box->byte_offset = idx;

	എങ്കിൽ (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	ആകുമ്പോൾ (child) {
		എങ്കിൽ (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	തിരികെ idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ഘടന content *c,
			  unsigned start_idx,
			  unsigned end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ഘടന rdw_info rdw;

	എങ്കിൽ (html->layout == ശൂന്യ) {
		തിരികെ NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ഘടന content *c,
			unsigned start_idx,
			unsigned end_idx,
			ഘടന selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	എങ്കിൽ (html->layout == ശൂന്യ) {
		തിരികെ NSERROR_INVALID;
	}

	തിരികെ selection_copy(html->layout,
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
html_textselection_get_end(ഘടന content *c, unsigned *end_idx)
{
	html_content *html = (html_content *)c;
	unsigned വേര്_idx;

	എങ്കിൽ (html->layout == ശൂന്യ) {
		തിരികെ NSERROR_INVALID;
	}

	വേര്_idx = 0;

	*end_idx = selection_label_subtree(html->layout, വേര്_idx);

	തിരികെ NSERROR_OK;
}
