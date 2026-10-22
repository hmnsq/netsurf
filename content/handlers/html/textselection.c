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

#समावेश <stdlib.h>

#समावेश "utils/errors.h"
#समावेश "utils/utils.h"
#समावेश "netsurf/types.h"
#समावेश "netsurf/plot_style.h"
#समावेश "desktop/selection.h"
#समावेश "desktop/save_text.h"

#समावेश "html/private.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/font.h"
#समावेश "html/textselection.h"

#घोषणा SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

काष्ठा rdw_info {
	bool inited;
	काष्ठा rect r;
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
अटल bool
selected_part(काष्ठा box *box,
	      अचिन्हित start_idx,
	      अचिन्हित end_idx,
	      अचिन्हित *start_offset,
	      अचिन्हित *end_offset)
{
	माप_प्रकार box_लंबाई = box->लंबाई + SPACE_LEN(box);

	अगर (box_लंबाई > 0) {
		अगर ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_लंबाई <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_लंबाई;
			वापस true;
		} अन्यथा अगर ((box->byte_offset + box_लंबाई > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			पूर्णांक offset = 0;
			पूर्णांक len;

			अगर (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_लंबाई - offset;

			अगर (box->byte_offset + box_लंबाई > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			वापस true;
		}
	}
	वापस false;
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
अटल nserror
coords_from_range(काष्ठा box *box,
		  अचिन्हित start_idx,
		  अचिन्हित end_idx,
		  काष्ठा rdw_info *rdwi,
		  bool do_marker)
{
	काष्ठा box *child;
	nserror res;

	निश्चित(box);

	/* If selection starts inside marker */
	अगर (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	अगर (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	अगर (box->byte_offset >= end_idx) {
		वापस NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	अगर ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_बायाँ ||
	       box->type == BOX_FLOAT_दायाँ) &&
	      !box->text)) {
		अचिन्हित start_off;
		अचिन्हित end_off;

		अगर (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			पूर्णांक width, height;
			पूर्णांक x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2];
			height = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];

			अगर ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			अगर (rdwi->inited) {
				अगर (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				अगर (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				अगर (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				अगर (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} अन्यथा {
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

	अगर (child) {
		काष्ठा box *अग्र = child->अग्र;

		जबतक (अग्र && अग्र->byte_offset < start_idx) {
			child = अग्र;
			अग्र = child->अग्र;
		}

		जबतक (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			काष्ठा box *अग्र = child->अग्र;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			अगर (res != NSERROR_OK) {
				वापस res;
			}

			child = अग्र;
		}
	}

	वापस NSERROR_OK;
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
अटल nserror
selection_copy_box(स्थिर अक्षर *text,
		   माप_प्रकार लंबाई,
		   काष्ठा box *box,
		   स्थिर css_unit_ctx *unit_len_ctx,
		   काष्ठा selection_string *handle,
		   स्थिर अक्षर *whitespace_text,
		   माप_प्रकार whitespace_लंबाई)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = शून्य;

	/* add any whitespace which precedes the text from this box */
	अगर (whitespace_text != शून्य &&
	    whitespace_लंबाई > 0) {
		अगर (!selection_string_append(whitespace_text,
					     whitespace_लंबाई,
					     false,
					     pstyle,
					     handle)) {
			वापस NSERROR_NOMEM;
		}
	}

	अगर (box != शून्य) {
		/* HTML */
		add_space = (box->space != 0);

		अगर (box->style != शून्य) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} अन्यथा {
			/* If there's no style, there must be no text */
			निश्चित(box->text == शून्य);
		}
	}

	/* add the text from this box */
	अगर (!selection_string_append(text, लंबाई, add_space, pstyle, handle)) {
		वापस NSERROR_NOMEM;
	}

	वापस NSERROR_OK;
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
अटल nserror
selection_copy(काष्ठा box *box,
	       स्थिर css_unit_ctx *unit_len_ctx,
	       अचिन्हित start_idx,
	       अचिन्हित end_idx,
	       काष्ठा selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	काष्ठा box *child;
	स्थिर अक्षर *whitespace_text = "";
	माप_प्रकार whitespace_लंबाई = 0;

	निश्चित(box);

	/* If selection starts inside marker */
	अगर (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	अगर (box->list_marker) {
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
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	अगर (box->byte_offset >= end_idx) {
		वापस NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	अगर (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_लंबाई);
	} अन्यथा {
		whitespace_text = शून्य;
	}

	अगर ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_बायाँ ||
	       box->type == BOX_FLOAT_दायाँ) &&
	      !box->text)) {
		अचिन्हित start_off;
		अचिन्हित end_off;

		अगर (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->लंबाई, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_लंबाई);
			अगर (res != NSERROR_OK) {
				वापस res;
			}
			अगर (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	अगर (child) {
		काष्ठा box *अग्र = child->अग्र;

		जबतक (अग्र && अग्र->byte_offset < start_idx) {
			child = अग्र;
			अग्र = child->अग्र;
		}

		जबतक (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			काष्ठा box *अग्र = child->अग्र;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			अगर (res != NSERROR_OK) {
				वापस res;
			}

			child = अग्र;
		}
	}

	वापस NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
अटल अचिन्हित selection_label_subtree(काष्ठा box *box, अचिन्हित idx)
{
	काष्ठा box *child;

	निश्चित(box != शून्य);

	child = box->children;

	box->byte_offset = idx;

	अगर (box->text) {
		idx += box->लंबाई + SPACE_LEN(box);
	}

	जबतक (child) {
		अगर (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->अग्र;
	}

	वापस idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(काष्ठा content *c,
			  अचिन्हित start_idx,
			  अचिन्हित end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	काष्ठा rdw_info rdw;

	अगर (html->layout == शून्य) {
		वापस NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	वापस NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(काष्ठा content *c,
			अचिन्हित start_idx,
			अचिन्हित end_idx,
			काष्ठा selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	अगर (html->layout == शून्य) {
		वापस NSERROR_INVALID;
	}

	वापस selection_copy(html->layout,
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
html_textselection_get_end(काष्ठा content *c, अचिन्हित *end_idx)
{
	html_content *html = (html_content *)c;
	अचिन्हित मूल_idx;

	अगर (html->layout == शून्य) {
		वापस NSERROR_INVALID;
	}

	मूल_idx = 0;

	*end_idx = selection_label_subtree(html->layout, मूल_idx);

	वापस NSERROR_OK;
}
