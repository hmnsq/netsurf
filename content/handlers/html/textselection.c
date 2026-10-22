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

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/save_text.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/textselection.h"

#ವ್ಯಾಖ್ಯಾನಿಸು SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ರಚನೆ rdw_info {
	bool inited;
	ರಚನೆ rect r;
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
ಸ್ಥಿರ bool
selected_part(ರಚನೆ box *box,
	      ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
	      ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
	      ಚಿಹ್ನೆರಹಿತ *ಆರಂಭ_ಸ್ಥಾನಾಂತರ,
	      ಚಿಹ್ನೆರಹಿತ *ಕೊನೆ_ಸ್ಥಾನಾಂತರ)
{
	size_t box_ಉದ್ದಳತೆ = box->ಉದ್ದಳತೆ + SPACE_LEN(box);

	ಆದರೆ (box_ಉದ್ದಳತೆ > 0) {
		ಆದರೆ ((box->byte_ಸ್ಥಾನಾಂತರ >= ಆರಂಭ_idx) &&
		    (box->byte_ಸ್ಥಾನಾಂತರ + box_ಉದ್ದಳತೆ <= ಕೊನೆ_idx)) {

			/* fully enclosed */
			*ಆರಂಭ_ಸ್ಥಾನಾಂತರ = 0;
			*ಕೊನೆ_ಸ್ಥಾನಾಂತರ = box_ಉದ್ದಳತೆ;
			ಹಿಂತಿರುಗಿ true;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((box->byte_ಸ್ಥಾನಾಂತರ + box_ಉದ್ದಳತೆ > ಆರಂಭ_idx) &&
			   (box->byte_ಸ್ಥಾನಾಂತರ < ಕೊನೆ_idx)) {
			/* partly enclosed */
			ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = 0;
			ಪೂರ್ಣಾಂಕ len;

			ಆದರೆ (box->byte_ಸ್ಥಾನಾಂತರ < ಆರಂಭ_idx) {
				ಸ್ಥಾನಾಂತರ = ಆರಂಭ_idx - box->byte_ಸ್ಥಾನಾಂತರ;
			}

			len = box_ಉದ್ದಳತೆ - ಸ್ಥಾನಾಂತರ;

			ಆದರೆ (box->byte_ಸ್ಥಾನಾಂತರ + box_ಉದ್ದಳತೆ > ಕೊನೆ_idx) {
				len = ಕೊನೆ_idx - (box->byte_ಸ್ಥಾನಾಂತರ + ಸ್ಥಾನಾಂತರ);
			}

			*ಆರಂಭ_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;
			*ಕೊನೆ_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ + len;

			ಹಿಂತಿರುಗಿ true;
		}
	}
	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ nserror
coords_from_range(ರಚನೆ box *box,
		  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
		  ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
		  ರಚನೆ rdw_info *rdwi,
		  bool do_marker)
{
	ರಚನೆ box *ಮಗು;
	nserror res;

	ಪ್ರತಿಜ್ಞಾ(box);

	/* If selection starts inside marker */
	ಆದರೆ (box->ಪೋಷಕ &&
	    box->ಪೋಷಕ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ಪೋಷಕ;
	}

	/* If box has a list marker */
	ಆದರೆ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					ಆರಂಭ_idx,
					ಕೊನೆ_idx,
					rdwi,
					true);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ಆದರೆ (box->byte_ಸ್ಥಾನಾಂತರ >= ಕೊನೆ_idx) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ಮಗು = box->children;

	ಆದರೆ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ಎಡ ||
	       box->type == BOX_FLOAT_ಬಲ) &&
	      !box->ಪಠ್ಯ)) {
		ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_off;
		ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_off;

		ಆದರೆ (selected_part(box, ಆರಂಭ_idx, ಕೊನೆ_idx, &ಆರಂಭ_off, &ಕೊನೆ_off)) {
			ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
			ಪೂರ್ಣಾಂಕ x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			ಅಗಲ = box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2];
			ಎತ್ತರ = box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM];

			ಆದರೆ ((box->type == BOX_ಪಠ್ಯ) &&
			    (box->space != 0)) {
				ಅಗಲ += box->space;
			}

			ಆದರೆ (rdwi->inited) {
				ಆದರೆ (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				ಆದರೆ (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				ಆದರೆ (x + ಅಗಲ > rdwi->r.x1) {
					rdwi->r.x1 = x + ಅಗಲ;
				}
				ಆದರೆ (y + ಎತ್ತರ > rdwi->r.y1) {
					rdwi->r.y1 = y + ಎತ್ತರ;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + ಅಗಲ;
				rdwi->r.y1 = y + ಎತ್ತರ;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ಆದರೆ (ಮಗು) {
		ರಚನೆ box *ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;

		ಆಗ (ಮುಂದಿನ && ಮುಂದಿನ->byte_ಸ್ಥಾನಾಂತರ < ಆರಂಭ_idx) {
			ಮಗು = ಮುಂದಿನ;
			ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;
		}

		ಆಗ (ಮಗು) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ರಚನೆ box *ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;

			res = coords_from_range(ಮಗು,
						ಆರಂಭ_idx,
						ಕೊನೆ_idx,
						rdwi,
						false);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}

			ಮಗು = ಮುಂದಿನ;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
selection_copy_box(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ,
		   size_t ಉದ್ದಳತೆ,
		   ರಚನೆ box *box,
		   ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		   ರಚನೆ selection_string *handle,
		   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *whitespace_ಪಠ್ಯ,
		   size_t whitespace_ಉದ್ದಳತೆ)
{
	bool ಕೂಡಿಸು_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = ಖಾಲಿ;

	/* add any whitespace which precedes the text from this box */
	ಆದರೆ (whitespace_ಪಠ್ಯ != ಖಾಲಿ &&
	    whitespace_ಉದ್ದಳತೆ > 0) {
		ಆದರೆ (!selection_string_append(whitespace_ಪಠ್ಯ,
					     whitespace_ಉದ್ದಳತೆ,
					     false,
					     pstyle,
					     handle)) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ಆದರೆ (box != ಖಾಲಿ) {
		/* HTML */
		ಕೂಡಿಸು_space = (box->space != 0);

		ಆದರೆ (box->style != ಖಾಲಿ) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* If there's no style, there must be no text */
			ಪ್ರತಿಜ್ಞಾ(box->ಪಠ್ಯ == ಖಾಲಿ);
		}
	}

	/* add the text from this box */
	ಆದರೆ (!selection_string_append(ಪಠ್ಯ, ಉದ್ದಳತೆ, ಕೂಡಿಸು_space, pstyle, handle)) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
selection_copy(ರಚನೆ box *box,
	       ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
	       ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
	       ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
	       ರಚನೆ selection_string *selstr,
	       save_ಪಠ್ಯ_whitespace *before,
	       bool *ಮೊದಲ,
	       bool do_marker)
{
	nserror res;
	ರಚನೆ box *ಮಗು;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *whitespace_ಪಠ್ಯ = "";
	size_t whitespace_ಉದ್ದಳತೆ = 0;

	ಪ್ರತಿಜ್ಞಾ(box);

	/* If selection starts inside marker */
	ಆದರೆ (box->ಪೋಷಕ &&
	    box->ಪೋಷಕ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ಪೋಷಕ;
	}

	/* If box has a list marker */
	ಆದರೆ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     ಆರಂಭ_idx,
				     ಕೊನೆ_idx,
				     selstr,
				     before,
				     ಮೊದಲ,
				     true);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ಆದರೆ (box->byte_ಸ್ಥಾನಾಂತರ >= ಕೊನೆ_idx) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ಮಗು = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	ಆದರೆ (before) {
		save_ಪಠ್ಯ_solve_whitespace(box,
					   ಮೊದಲ,
					   before,
					   &whitespace_ಪಠ್ಯ,
					   &whitespace_ಉದ್ದಳತೆ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		whitespace_ಪಠ್ಯ = ಖಾಲಿ;
	}

	ಆದರೆ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ಎಡ ||
	       box->type == BOX_FLOAT_ಬಲ) &&
	      !box->ಪಠ್ಯ)) {
		ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_off;
		ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_off;

		ಆದರೆ (selected_part(box, ಆರಂಭ_idx, ಕೊನೆ_idx, &ಆರಂಭ_off, &ಕೊನೆ_off)) {
			res = selection_copy_box(box->ಪಠ್ಯ + ಆರಂಭ_off,
						 min(box->ಉದ್ದಳತೆ, ಕೊನೆ_off) - ಆರಂಭ_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_ಪಠ್ಯ,
						 whitespace_ಉದ್ದಳತೆ);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}
			ಆದರೆ (before) {
				*ಮೊದಲ = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ಆದರೆ (ಮಗು) {
		ರಚನೆ box *ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;

		ಆಗ (ಮುಂದಿನ && ಮುಂದಿನ->byte_ಸ್ಥಾನಾಂತರ < ಆರಂಭ_idx) {
			ಮಗು = ಮುಂದಿನ;
			ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;
		}

		ಆಗ (ಮಗು) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ರಚನೆ box *ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;

			res = selection_copy(ಮಗು,
					     unit_len_ctx,
					     ಆರಂಭ_idx,
					     ಕೊನೆ_idx,
					     selstr,
					     before,
					     ಮೊದಲ,
					     false);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}

			ಮಗು = ಮುಂದಿನ;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ selection_label_subtree(ರಚನೆ box *box, ಚಿಹ್ನೆರಹಿತ idx)
{
	ರಚನೆ box *ಮಗು;

	ಪ್ರತಿಜ್ಞಾ(box != ಖಾಲಿ);

	ಮಗು = box->children;

	box->byte_ಸ್ಥಾನಾಂತರ = idx;

	ಆದರೆ (box->ಪಠ್ಯ) {
		idx += box->ಉದ್ದಳತೆ + SPACE_LEN(box);
	}

	ಆಗ (ಮಗು) {
		ಆದರೆ (ಮಗು->list_marker) {
			idx = selection_label_subtree(ಮಗು->list_marker, idx);
		}

		idx = selection_label_subtree(ಮಗು, idx);
		ಮಗು = ಮಗು->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ರಚನೆ content *c,
			  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			  ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ರಚನೆ rdw_info rdw;

	ಆದರೆ (html->layout == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, ಆರಂಭ_idx, ಕೊನೆ_idx, &rdw, false);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ರಚನೆ content *c,
			ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
			ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
			ರಚನೆ selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_ಪಠ್ಯ_whitespace before = WHITESPACE_NONE;
	bool ಮೊದಲ = true;

	ಆದರೆ (html->layout == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಹಿಂತಿರುಗಿ selection_copy(html->layout,
			      &html->unit_len_ctx,
			      ಆರಂಭ_idx,
			      ಕೊನೆ_idx,
			      selstr,
			      &before,
			      &ಮೊದಲ,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_ಕೊನೆ(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ *ಕೊನೆ_idx)
{
	html_content *html = (html_content *)c;
	ಚಿಹ್ನೆರಹಿತ ಬೇರು_idx;

	ಆದರೆ (html->layout == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಬೇರು_idx = 0;

	*ಕೊನೆ_idx = selection_label_subtree(html->layout, ಬೇರು_idx);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}
