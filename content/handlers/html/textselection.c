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

#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "netsurf/plot_style.h"
#अन्तर्भाव "desktop/selection.h"
#अन्तर्भाव "desktop/save_text.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/textselection.h"

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

संरचना rdw_info {
	bool inited;
	संरचना rect r;
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
स्थिर bool
selected_part(संरचना box *box,
	      अचिन्हित start_idx,
	      अचिन्हित end_idx,
	      अचिन्हित *start_offset,
	      अचिन्हित *end_offset)
{
	माप_प्रकार box_length = box->length + SPACE_LEN(box);

	जर (box_length > 0) {
		जर ((box->byte_offset >= start_idx) &&
		    (box->byte_offset + box_length <= end_idx)) {

			/* fully enclosed */
			*start_offset = 0;
			*end_offset = box_length;
			परत true;
		} अन्यथा जर ((box->byte_offset + box_length > start_idx) &&
			   (box->byte_offset < end_idx)) {
			/* partly enclosed */
			पूर्णांक offset = 0;
			पूर्णांक len;

			जर (box->byte_offset < start_idx) {
				offset = start_idx - box->byte_offset;
			}

			len = box_length - offset;

			जर (box->byte_offset + box_length > end_idx) {
				len = end_idx - (box->byte_offset + offset);
			}

			*start_offset = offset;
			*end_offset = offset + len;

			परत true;
		}
	}
	परत false;
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
स्थिर nserror
coords_from_range(संरचना box *box,
		  अचिन्हित start_idx,
		  अचिन्हित end_idx,
		  संरचना rdw_info *rdwi,
		  bool do_marker)
{
	संरचना box *child;
	nserror res;

	प्रतिज्ञा(box);

	/* If selection starts inside marker */
	जर (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	जर (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					end_idx,
					rdwi,
					true);
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	जर (box->byte_offset >= end_idx) {
		परत NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	जर ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_डावा ||
	       box->type == BOX_FLOAT_उजवा) &&
	      !box->text)) {
		अचिन्हित start_off;
		अचिन्हित end_off;

		जर (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			पूर्णांक width, height;
			पूर्णांक x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			width = box->padding[डावा_2] + box->width + box->padding[उजवा_2];
			height = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];

			जर ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				width += box->space;
			}

			जर (rdwi->inited) {
				जर (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				जर (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				जर (x + width > rdwi->r.x1) {
					rdwi->r.x1 = x + width;
				}
				जर (y + height > rdwi->r.y1) {
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

	जर (child) {
		संरचना box *next = child->next;

		पर्यंत (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		पर्यंत (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			संरचना box *next = child->next;

			res = coords_from_range(child,
						start_idx,
						end_idx,
						rdwi,
						false);
			जर (res != NSERROR_OK) {
				परत res;
			}

			child = next;
		}
	}

	परत NSERROR_OK;
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
स्थिर nserror
selection_copy_box(स्थिरांक अक्षर *text,
		   माप_प्रकार length,
		   संरचना box *box,
		   स्थिरांक css_unit_ctx *unit_len_ctx,
		   संरचना selection_string *handle,
		   स्थिरांक अक्षर *whitespace_text,
		   माप_प्रकार whitespace_length)
{
	bool add_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = रिक्त;

	/* add any whitespace which precedes the text from this box */
	जर (whitespace_text != रिक्त &&
	    whitespace_length > 0) {
		जर (!selection_string_append(whitespace_text,
					     whitespace_length,
					     false,
					     pstyle,
					     handle)) {
			परत NSERROR_NOMEM;
		}
	}

	जर (box != रिक्त) {
		/* HTML */
		add_space = (box->space != 0);

		जर (box->style != रिक्त) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} अन्यथा {
			/* If there's no style, there must be no text */
			प्रतिज्ञा(box->text == रिक्त);
		}
	}

	/* add the text from this box */
	जर (!selection_string_append(text, length, add_space, pstyle, handle)) {
		परत NSERROR_NOMEM;
	}

	परत NSERROR_OK;
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
स्थिर nserror
selection_copy(संरचना box *box,
	       स्थिरांक css_unit_ctx *unit_len_ctx,
	       अचिन्हित start_idx,
	       अचिन्हित end_idx,
	       संरचना selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	संरचना box *child;
	स्थिरांक अक्षर *whitespace_text = "";
	माप_प्रकार whitespace_length = 0;

	प्रतिज्ञा(box);

	/* If selection starts inside marker */
	जर (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	जर (box->list_marker) {
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
		जर (res != NSERROR_OK) {
			परत res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	जर (box->byte_offset >= end_idx) {
		परत NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	जर (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_length);
	} अन्यथा {
		whitespace_text = रिक्त;
	}

	जर ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_डावा ||
	       box->type == BOX_FLOAT_उजवा) &&
	      !box->text)) {
		अचिन्हित start_off;
		अचिन्हित end_off;

		जर (selected_part(box, start_idx, end_idx, &start_off, &end_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->length, end_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_length);
			जर (res != NSERROR_OK) {
				परत res;
			}
			जर (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	जर (child) {
		संरचना box *next = child->next;

		पर्यंत (next && next->byte_offset < start_idx) {
			child = next;
			next = child->next;
		}

		पर्यंत (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			संरचना box *next = child->next;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     end_idx,
					     selstr,
					     before,
					     first,
					     false);
			जर (res != NSERROR_OK) {
				परत res;
			}

			child = next;
		}
	}

	परत NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
स्थिर अचिन्हित selection_label_subtree(संरचना box *box, अचिन्हित idx)
{
	संरचना box *child;

	प्रतिज्ञा(box != रिक्त);

	child = box->children;

	box->byte_offset = idx;

	जर (box->text) {
		idx += box->length + SPACE_LEN(box);
	}

	पर्यंत (child) {
		जर (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->next;
	}

	परत idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(संरचना content *c,
			  अचिन्हित start_idx,
			  अचिन्हित end_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	संरचना rdw_info rdw;

	जर (html->layout == रिक्त) {
		परत NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, end_idx, &rdw, false);
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	परत NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(संरचना content *c,
			अचिन्हित start_idx,
			अचिन्हित end_idx,
			संरचना selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	जर (html->layout == रिक्त) {
		परत NSERROR_INVALID;
	}

	परत selection_copy(html->layout,
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
html_textselection_get_end(संरचना content *c, अचिन्हित *end_idx)
{
	html_content *html = (html_content *)c;
	अचिन्हित मूळ_idx;

	जर (html->layout == रिक्त) {
		परत NSERROR_INVALID;
	}

	मूळ_idx = 0;

	*end_idx = selection_label_subtree(html->layout, मूळ_idx);

	परत NSERROR_OK;
}
