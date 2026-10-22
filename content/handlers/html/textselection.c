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

#परिभाषय SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

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
	      अचिह्नित start_idx,
	      अचिह्नित अन्तः_idx,
	      अचिह्नित *start_विस्थापनम्,
	      अचिह्नित *अन्तः_विस्थापनम्)
{
	size_t box_दीर्घता = box->दीर्घता + SPACE_LEN(box);

	यदि (box_दीर्घता > 0) {
		यदि ((box->byte_विस्थापनम् >= start_idx) &&
		    (box->byte_विस्थापनम् + box_दीर्घता <= अन्तः_idx)) {

			/* fully enclosed */
			*start_विस्थापनम् = 0;
			*अन्तः_विस्थापनम् = box_दीर्घता;
			प्रत्यागम true;
		} अन्यथा यदि ((box->byte_विस्थापनम् + box_दीर्घता > start_idx) &&
			   (box->byte_विस्थापनम् < अन्तः_idx)) {
			/* partly enclosed */
			पूर्णाङ्क विस्थापनम् = 0;
			पूर्णाङ्क len;

			यदि (box->byte_विस्थापनम् < start_idx) {
				विस्थापनम् = start_idx - box->byte_विस्थापनम्;
			}

			len = box_दीर्घता - विस्थापनम्;

			यदि (box->byte_विस्थापनम् + box_दीर्घता > अन्तः_idx) {
				len = अन्तः_idx - (box->byte_विस्थापनम् + विस्थापनम्);
			}

			*start_विस्थापनम् = विस्थापनम्;
			*अन्तः_विस्थापनम् = विस्थापनम् + len;

			प्रत्यागम true;
		}
	}
	प्रत्यागम false;
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
		  अचिह्नित start_idx,
		  अचिह्नित अन्तः_idx,
		  संरचना rdw_info *rdwi,
		  bool do_marker)
{
	संरचना box *child;
	nserror res;

	प्रतिज्ञा(box);

	/* If selection starts inside marker */
	यदि (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	यदि (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					start_idx,
					अन्तः_idx,
					rdwi,
					true);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	यदि (box->byte_विस्थापनम् >= अन्तः_idx) {
		प्रत्यागम NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	यदि ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_वामः ||
	       box->type == BOX_FLOAT_दक्षिणः) &&
	      !box->text)) {
		अचिह्नित start_off;
		अचिह्नित अन्तः_off;

		यदि (selected_part(box, start_idx, अन्तः_idx, &start_off, &अन्तः_off)) {
			पूर्णाङ्क विस्तारः, height;
			पूर्णाङ्क x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			विस्तारः = box->padding[वामः_2] + box->विस्तारः + box->padding[दक्षिणः_2];
			height = box->padding[शिखरम्_2] + box->height + box->padding[BOTTOM];

			यदि ((box->type == BOX_TEXT) &&
			    (box->space != 0)) {
				विस्तारः += box->space;
			}

			यदि (rdwi->inited) {
				यदि (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				यदि (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				यदि (x + विस्तारः > rdwi->r.x1) {
					rdwi->r.x1 = x + विस्तारः;
				}
				यदि (y + height > rdwi->r.y1) {
					rdwi->r.y1 = y + height;
				}
			} अन्यथा {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + विस्तारः;
				rdwi->r.y1 = y + height;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	यदि (child) {
		संरचना box *अग्र = child->अग्र;

		यावत् (अग्र && अग्र->byte_विस्थापनम् < start_idx) {
			child = अग्र;
			अग्र = child->अग्र;
		}

		यावत् (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			संरचना box *अग्र = child->अग्र;

			res = coords_from_range(child,
						start_idx,
						अन्तः_idx,
						rdwi,
						false);
			यदि (res != NSERROR_OK) {
				प्रत्यागम res;
			}

			child = अग्र;
		}
	}

	प्रत्यागम NSERROR_OK;
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
selection_copy_box(ध्रुव वर्ण *text,
		   size_t दीर्घता,
		   संरचना box *box,
		   ध्रुव css_unit_ctx *unit_len_ctx,
		   संरचना selection_string *handle,
		   ध्रुव वर्ण *whitespace_text,
		   size_t whitespace_दीर्घता)
{
	bool सङ्कलय_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = रिक्त;

	/* add any whitespace which precedes the text from this box */
	यदि (whitespace_text != रिक्त &&
	    whitespace_दीर्घता > 0) {
		यदि (!selection_string_append(whitespace_text,
					     whitespace_दीर्घता,
					     false,
					     pstyle,
					     handle)) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	यदि (box != रिक्त) {
		/* HTML */
		सङ्कलय_space = (box->space != 0);

		यदि (box->style != रिक्त) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} अन्यथा {
			/* If there's no style, there must be no text */
			प्रतिज्ञा(box->text == रिक्त);
		}
	}

	/* add the text from this box */
	यदि (!selection_string_append(text, दीर्घता, सङ्कलय_space, pstyle, handle)) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रत्यागम NSERROR_OK;
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
	       ध्रुव css_unit_ctx *unit_len_ctx,
	       अचिह्नित start_idx,
	       अचिह्नित अन्तः_idx,
	       संरचना selection_string *selstr,
	       save_text_whitespace *before,
	       bool *first,
	       bool do_marker)
{
	nserror res;
	संरचना box *child;
	ध्रुव वर्ण *whitespace_text = "";
	size_t whitespace_दीर्घता = 0;

	प्रतिज्ञा(box);

	/* If selection starts inside marker */
	यदि (box->parent &&
	    box->parent->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->parent;
	}

	/* If box has a list marker */
	यदि (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     start_idx,
				     अन्तः_idx,
				     selstr,
				     before,
				     first,
				     true);
		यदि (res != NSERROR_OK) {
			प्रत्यागम res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	यदि (box->byte_विस्थापनम् >= अन्तः_idx) {
		प्रत्यागम NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	child = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	यदि (before) {
		save_text_solve_whitespace(box,
					   first,
					   before,
					   &whitespace_text,
					   &whitespace_दीर्घता);
	} अन्यथा {
		whitespace_text = रिक्त;
	}

	यदि ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_वामः ||
	       box->type == BOX_FLOAT_दक्षिणः) &&
	      !box->text)) {
		अचिह्नित start_off;
		अचिह्नित अन्तः_off;

		यदि (selected_part(box, start_idx, अन्तः_idx, &start_off, &अन्तः_off)) {
			res = selection_copy_box(box->text + start_off,
						 min(box->दीर्घता, अन्तः_off) - start_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_text,
						 whitespace_दीर्घता);
			यदि (res != NSERROR_OK) {
				प्रत्यागम res;
			}
			यदि (before) {
				*first = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	यदि (child) {
		संरचना box *अग्र = child->अग्र;

		यावत् (अग्र && अग्र->byte_विस्थापनम् < start_idx) {
			child = अग्र;
			अग्र = child->अग्र;
		}

		यावत् (child) {
			/* read before calling the handler in case it modifies
			 * the tree */
			संरचना box *अग्र = child->अग्र;

			res = selection_copy(child,
					     unit_len_ctx,
					     start_idx,
					     अन्तः_idx,
					     selstr,
					     before,
					     first,
					     false);
			यदि (res != NSERROR_OK) {
				प्रत्यागम res;
			}

			child = अग्र;
		}
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
स्थिर अचिह्नित selection_label_subtree(संरचना box *box, अचिह्नित idx)
{
	संरचना box *child;

	प्रतिज्ञा(box != रिक्त);

	child = box->children;

	box->byte_विस्थापनम् = idx;

	यदि (box->text) {
		idx += box->दीर्घता + SPACE_LEN(box);
	}

	यावत् (child) {
		यदि (child->list_marker) {
			idx = selection_label_subtree(child->list_marker, idx);
		}

		idx = selection_label_subtree(child, idx);
		child = child->अग्र;
	}

	प्रत्यागम idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(संरचना content *c,
			  अचिह्नित start_idx,
			  अचिह्नित अन्तः_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	संरचना rdw_info rdw;

	यदि (html->layout == रिक्त) {
		प्रत्यागम NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, start_idx, अन्तः_idx, &rdw, false);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(संरचना content *c,
			अचिह्नित start_idx,
			अचिह्नित अन्तः_idx,
			संरचना selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_text_whitespace before = WHITESPACE_NONE;
	bool first = true;

	यदि (html->layout == रिक्त) {
		प्रत्यागम NSERROR_INVALID;
	}

	प्रत्यागम selection_copy(html->layout,
			      &html->unit_len_ctx,
			      start_idx,
			      अन्तः_idx,
			      selstr,
			      &before,
			      &first,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_अन्तः(संरचना content *c, अचिह्नित *अन्तः_idx)
{
	html_content *html = (html_content *)c;
	अचिह्नित मूलम्_idx;

	यदि (html->layout == रिक्त) {
		प्रत्यागम NSERROR_INVALID;
	}

	मूलम्_idx = 0;

	*अन्तः_idx = selection_label_subtree(html->layout, मूलम्_idx);

	प्रत्यागम NSERROR_OK;
}
