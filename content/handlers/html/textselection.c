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

#تعریف_کړه SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

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
	      بینښه پیل_idx,
	      بینښه پای_ټکی_idx,
	      بینښه *پیل_دننه_واټن,
	      بینښه *پای_ټکی_دننه_واټن)
{
	size_t box_اوږدتیا = box->اوږدتیا + SPACE_LEN(box);

	إذا (box_اوږدتیا > 0) {
		إذا ((box->byte_دننه_واټن >= پیل_idx) &&
		    (box->byte_دننه_واټن + box_اوږدتیا <= پای_ټکی_idx)) {

			/* fully enclosed */
			*پیل_دننه_واټن = 0;
			*پای_ټکی_دننه_واټن = box_اوږدتیا;
			أرجع true;
		} وإلا إذا ((box->byte_دننه_واټن + box_اوږدتیا > پیل_idx) &&
			   (box->byte_دننه_واټن < پای_ټکی_idx)) {
			/* partly enclosed */
			صحيح دننه_واټن = 0;
			صحيح len;

			إذا (box->byte_دننه_واټن < پیل_idx) {
				دننه_واټن = پیل_idx - box->byte_دننه_واټن;
			}

			len = box_اوږدتیا - دننه_واټن;

			إذا (box->byte_دننه_واټن + box_اوږدتیا > پای_ټکی_idx) {
				len = پای_ټکی_idx - (box->byte_دننه_واټن + دننه_واټن);
			}

			*پیل_دننه_واټن = دننه_واټن;
			*پای_ټکی_دننه_واټن = دننه_واټن + len;

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
		  بینښه پیل_idx,
		  بینښه پای_ټکی_idx,
		  بنية rdw_info *rdwi,
		  bool do_marker)
{
	بنية box *ماشوم;
	nserror res;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->پلار &&
	    box->پلار->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->پلار;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					پیل_idx,
					پای_ټکی_idx,
					rdwi,
					true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_دننه_واټن >= پای_ټکی_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ماشوم = box->children;

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_کیڼ ||
	       box->type == BOX_FLOAT_ښی) &&
	      !box->متن)) {
		بینښه پیل_off;
		بینښه پای_ټکی_off;

		إذا (selected_part(box, پیل_idx, پای_ټکی_idx, &پیل_off, &پای_ټکی_off)) {
			صحيح پلنوالی, لوړوالی;
			صحيح x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			پلنوالی = box->padding[کیڼ_2] + box->پلنوالی + box->padding[ښی_2];
			لوړوالی = box->padding[سر_2] + box->لوړوالی + box->padding[BOTTOM];

			إذا ((box->type == BOX_متن) &&
			    (box->space != 0)) {
				پلنوالی += box->space;
			}

			إذا (rdwi->inited) {
				إذا (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				إذا (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				إذا (x + پلنوالی > rdwi->r.x1) {
					rdwi->r.x1 = x + پلنوالی;
				}
				إذا (y + لوړوالی > rdwi->r.y1) {
					rdwi->r.y1 = y + لوړوالی;
				}
			} وإلا {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + پلنوالی;
				rdwi->r.y1 = y + لوړوالی;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (ماشوم) {
		بنية box *بل = ماشوم->بل;

		طالما (بل && بل->byte_دننه_واټن < پیل_idx) {
			ماشوم = بل;
			بل = ماشوم->بل;
		}

		طالما (ماشوم) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *بل = ماشوم->بل;

			res = coords_from_range(ماشوم,
						پیل_idx,
						پای_ټکی_idx,
						rdwi,
						false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			ماشوم = بل;
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
		   size_t اوږدتیا,
		   بنية box *box,
		   ثابت css_unit_ctx *unit_len_ctx,
		   بنية selection_string *handle,
		   ثابت حرف *whitespace_متن,
		   size_t whitespace_اوږدتیا)
{
	bool جمع_کړه_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = عدم;

	/* add any whitespace which precedes the text from this box */
	إذا (whitespace_متن != عدم &&
	    whitespace_اوږدتیا > 0) {
		إذا (!selection_string_append(whitespace_متن,
					     whitespace_اوږدتیا,
					     false,
					     pstyle,
					     handle)) {
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (box != عدم) {
		/* HTML */
		جمع_کړه_space = (box->space != 0);

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
	إذا (!selection_string_append(متن, اوږدتیا, جمع_کړه_space, pstyle, handle)) {
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
	       بینښه پیل_idx,
	       بینښه پای_ټکی_idx,
	       بنية selection_string *selstr,
	       save_متن_whitespace *before,
	       bool *لومړی,
	       bool do_marker)
{
	nserror res;
	بنية box *ماشوم;
	ثابت حرف *whitespace_متن = "";
	size_t whitespace_اوږدتیا = 0;

	أكد(box);

	/* If selection starts inside marker */
	إذا (box->پلار &&
	    box->پلار->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->پلار;
	}

	/* If box has a list marker */
	إذا (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     پیل_idx,
				     پای_ټکی_idx,
				     selstr,
				     before,
				     لومړی,
				     true);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	إذا (box->byte_دننه_واټن >= پای_ټکی_idx) {
		أرجع NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ماشوم = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	إذا (before) {
		save_متن_solve_whitespace(box,
					   لومړی,
					   before,
					   &whitespace_متن,
					   &whitespace_اوږدتیا);
	} وإلا {
		whitespace_متن = عدم;
	}

	إذا ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_کیڼ ||
	       box->type == BOX_FLOAT_ښی) &&
	      !box->متن)) {
		بینښه پیل_off;
		بینښه پای_ټکی_off;

		إذا (selected_part(box, پیل_idx, پای_ټکی_idx, &پیل_off, &پای_ټکی_off)) {
			res = selection_copy_box(box->متن + پیل_off,
						 min(box->اوږدتیا, پای_ټکی_off) - پیل_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_متن,
						 whitespace_اوږدتیا);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
			إذا (before) {
				*لومړی = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	إذا (ماشوم) {
		بنية box *بل = ماشوم->بل;

		طالما (بل && بل->byte_دننه_واټن < پیل_idx) {
			ماشوم = بل;
			بل = ماشوم->بل;
		}

		طالما (ماشوم) {
			/* read before calling the handler in case it modifies
			 * the tree */
			بنية box *بل = ماشوم->بل;

			res = selection_copy(ماشوم,
					     unit_len_ctx,
					     پیل_idx,
					     پای_ټکی_idx,
					     selstr,
					     before,
					     لومړی,
					     false);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			ماشوم = بل;
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
ساكن بینښه selection_label_subtree(بنية box *box, بینښه idx)
{
	بنية box *ماشوم;

	أكد(box != عدم);

	ماشوم = box->children;

	box->byte_دننه_واټن = idx;

	إذا (box->متن) {
		idx += box->اوږدتیا + SPACE_LEN(box);
	}

	طالما (ماشوم) {
		إذا (ماشوم->list_marker) {
			idx = selection_label_subtree(ماشوم->list_marker, idx);
		}

		idx = selection_label_subtree(ماشوم, idx);
		ماشوم = ماشوم->بل;
	}

	أرجع idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(بنية content *c,
			  بینښه پیل_idx,
			  بینښه پای_ټکی_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	بنية rdw_info rdw;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, پیل_idx, پای_ټکی_idx, &rdw, false);
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
			بینښه پیل_idx,
			بینښه پای_ټکی_idx,
			بنية selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_متن_whitespace before = WHITESPACE_NONE;
	bool لومړی = true;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	أرجع selection_copy(html->layout,
			      &html->unit_len_ctx,
			      پیل_idx,
			      پای_ټکی_idx,
			      selstr,
			      &before,
			      &لومړی,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_پای_ټکی(بنية content *c, بینښه *پای_ټکی_idx)
{
	html_content *html = (html_content *)c;
	بینښه ریښه_idx;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	ریښه_idx = 0;

	*پای_ټکی_idx = selection_label_subtree(html->layout, ریښه_idx);

	أرجع NSERROR_OK;
}
