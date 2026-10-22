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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plot_style.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/save_text.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/textselection.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPACE_LEN(b) ((b->space == 0) ? 0 : 1)

ਬਣਤਰ rdw_info {
	bool inited;
	ਬਣਤਰ rect r;
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
ਸਥਿਰ bool
selected_part(ਬਣਤਰ box *box,
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
	      ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸ਼ੁਰੂ_ਵਿੱਥ,
	      ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸਿਰਾ_ਵਿੱਥ)
{
	size_t box_ਲੰਮਾਈ = box->ਲੰਮਾਈ + SPACE_LEN(box);

	ਜੇ (box_ਲੰਮਾਈ > 0) {
		ਜੇ ((box->byte_ਵਿੱਥ >= ਸ਼ੁਰੂ_idx) &&
		    (box->byte_ਵਿੱਥ + box_ਲੰਮਾਈ <= ਸਿਰਾ_idx)) {

			/* fully enclosed */
			*ਸ਼ੁਰੂ_ਵਿੱਥ = 0;
			*ਸਿਰਾ_ਵਿੱਥ = box_ਲੰਮਾਈ;
			ਵਾਪਸ true;
		} ਨਹੀਂਤਾਂ ਜੇ ((box->byte_ਵਿੱਥ + box_ਲੰਮਾਈ > ਸ਼ੁਰੂ_idx) &&
			   (box->byte_ਵਿੱਥ < ਸਿਰਾ_idx)) {
			/* partly enclosed */
			ਪੂਰਨ ਵਿੱਥ = 0;
			ਪੂਰਨ len;

			ਜੇ (box->byte_ਵਿੱਥ < ਸ਼ੁਰੂ_idx) {
				ਵਿੱਥ = ਸ਼ੁਰੂ_idx - box->byte_ਵਿੱਥ;
			}

			len = box_ਲੰਮਾਈ - ਵਿੱਥ;

			ਜੇ (box->byte_ਵਿੱਥ + box_ਲੰਮਾਈ > ਸਿਰਾ_idx) {
				len = ਸਿਰਾ_idx - (box->byte_ਵਿੱਥ + ਵਿੱਥ);
			}

			*ਸ਼ੁਰੂ_ਵਿੱਥ = ਵਿੱਥ;
			*ਸਿਰਾ_ਵਿੱਥ = ਵਿੱਥ + len;

			ਵਾਪਸ true;
		}
	}
	ਵਾਪਸ false;
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
ਸਥਿਰ nserror
coords_from_range(ਬਣਤਰ box *box,
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
		  ਬਣਤਰ rdw_info *rdwi,
		  bool do_marker)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	nserror res;

	ਪ੍ਰਤਿਜ੍ਞਾ(box);

	/* If selection starts inside marker */
	ਜੇ (box->ਮਾਪਾ &&
	    box->ਮਾਪਾ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ਮਾਪਾ;
	}

	/* If box has a list marker */
	ਜੇ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = coords_from_range(box->list_marker,
					ਸ਼ੁਰੂ_idx,
					ਸਿਰਾ_idx,
					rdwi,
					true);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ਜੇ (box->byte_ਵਿੱਥ >= ਸਿਰਾ_idx) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ਬੱਚਾ = box->children;

	ਜੇ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ਖੱਬਾ ||
	       box->type == BOX_FLOAT_ਸੱਜਾ) &&
	      !box->ਪਾਠ)) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_off;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_off;

		ਜੇ (selected_part(box, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, &ਸ਼ੁਰੂ_off, &ਸਿਰਾ_off)) {
			ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
			ਪੂਰਨ x, y;

			/**
			 * \todo it should be possible to reduce the redrawn
			 *        area using the offsets
			 */
			box_coords(box, &x, &y);

			ਚੌੜਾਈ = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
			ਉਚਾਈ = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM];

			ਜੇ ((box->type == BOX_ਪਾਠ) &&
			    (box->space != 0)) {
				ਚੌੜਾਈ += box->space;
			}

			ਜੇ (rdwi->inited) {
				ਜੇ (x < rdwi->r.x0) {
					rdwi->r.x0 = x;
				}
				ਜੇ (y < rdwi->r.y0) {
					rdwi->r.y0 = y;
				}
				ਜੇ (x + ਚੌੜਾਈ > rdwi->r.x1) {
					rdwi->r.x1 = x + ਚੌੜਾਈ;
				}
				ਜੇ (y + ਉਚਾਈ > rdwi->r.y1) {
					rdwi->r.y1 = y + ਉਚਾਈ;
				}
			} ਨਹੀਂਤਾਂ {
				rdwi->inited = true;
				rdwi->r.x0 = x;
				rdwi->r.y0 = y;
				rdwi->r.x1 = x + ਚੌੜਾਈ;
				rdwi->r.y1 = y + ਉਚਾਈ;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ਜੇ (ਬੱਚਾ) {
		ਬਣਤਰ box *ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;

		ਜਦੋਂ (ਅਗਲਾ && ਅਗਲਾ->byte_ਵਿੱਥ < ਸ਼ੁਰੂ_idx) {
			ਬੱਚਾ = ਅਗਲਾ;
			ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;
		}

		ਜਦੋਂ (ਬੱਚਾ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ਬਣਤਰ box *ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;

			res = coords_from_range(ਬੱਚਾ,
						ਸ਼ੁਰੂ_idx,
						ਸਿਰਾ_idx,
						rdwi,
						false);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}

			ਬੱਚਾ = ਅਗਲਾ;
		}
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
selection_copy_box(ਅਚਲ ਅੱਖਰ *ਪਾਠ,
		   size_t ਲੰਮਾਈ,
		   ਬਣਤਰ box *box,
		   ਅਚਲ css_unit_ctx *unit_len_ctx,
		   ਬਣਤਰ selection_string *handle,
		   ਅਚਲ ਅੱਖਰ *whitespace_ਪਾਠ,
		   size_t whitespace_ਲੰਮਾਈ)
{
	bool ਜੋੜੋ_space = false;
	plot_font_style_t style;
	plot_font_style_t *pstyle = ਨਿਰਾ;

	/* add any whitespace which precedes the text from this box */
	ਜੇ (whitespace_ਪਾਠ != ਨਿਰਾ &&
	    whitespace_ਲੰਮਾਈ > 0) {
		ਜੇ (!selection_string_append(whitespace_ਪਾਠ,
					     whitespace_ਲੰਮਾਈ,
					     false,
					     pstyle,
					     handle)) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ਜੇ (box != ਨਿਰਾ) {
		/* HTML */
		ਜੋੜੋ_space = (box->space != 0);

		ਜੇ (box->style != ਨਿਰਾ) {
			/* Override default font style */
			font_plot_style_from_css(unit_len_ctx, box->style, &style);
			pstyle = &style;
		} ਨਹੀਂਤਾਂ {
			/* If there's no style, there must be no text */
			ਪ੍ਰਤਿਜ੍ਞਾ(box->ਪਾਠ == ਨਿਰਾ);
		}
	}

	/* add the text from this box */
	ਜੇ (!selection_string_append(ਪਾਠ, ਲੰਮਾਈ, ਜੋੜੋ_space, pstyle, handle)) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
selection_copy(ਬਣਤਰ box *box,
	       ਅਚਲ css_unit_ctx *unit_len_ctx,
	       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
	       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
	       ਬਣਤਰ selection_string *selstr,
	       save_ਪਾਠ_whitespace *before,
	       bool *ਪਹਿਲਾ,
	       bool do_marker)
{
	nserror res;
	ਬਣਤਰ box *ਬੱਚਾ;
	ਅਚਲ ਅੱਖਰ *whitespace_ਪਾਠ = "";
	size_t whitespace_ਲੰਮਾਈ = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(box);

	/* If selection starts inside marker */
	ਜੇ (box->ਮਾਪਾ &&
	    box->ਮਾਪਾ->list_marker == box &&
	    !do_marker) {
		/* set box to main list element */
		box = box->ਮਾਪਾ;
	}

	/* If box has a list marker */
	ਜੇ (box->list_marker) {
		/* do the marker box before continuing with the rest of the
		 * list element */
		res = selection_copy(box->list_marker,
				     unit_len_ctx,
				     ਸ਼ੁਰੂ_idx,
				     ਸਿਰਾ_idx,
				     selstr,
				     before,
				     ਪਹਿਲਾ,
				     true);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	/* we can prune this subtree, it's after the selection */
	ਜੇ (box->byte_ਵਿੱਥ >= ਸਿਰਾ_idx) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* read before calling the handler in case it modifies the tree */
	ਬੱਚਾ = box->children;

	/* If nicely formatted output of the selected text is required, work
	 * out what whitespace should be placed before the next bit of text */
	ਜੇ (before) {
		save_ਪਾਠ_solve_whitespace(box,
					   ਪਹਿਲਾ,
					   before,
					   &whitespace_ਪਾਠ,
					   &whitespace_ਲੰਮਾਈ);
	} ਨਹੀਂਤਾਂ {
		whitespace_ਪਾਠ = ਨਿਰਾ;
	}

	ਜੇ ((box->type != BOX_BR) &&
	    !((box->type == BOX_FLOAT_ਖੱਬਾ ||
	       box->type == BOX_FLOAT_ਸੱਜਾ) &&
	      !box->ਪਾਠ)) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_off;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_off;

		ਜੇ (selected_part(box, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, &ਸ਼ੁਰੂ_off, &ਸਿਰਾ_off)) {
			res = selection_copy_box(box->ਪਾਠ + ਸ਼ੁਰੂ_off,
						 min(box->ਲੰਮਾਈ, ਸਿਰਾ_off) - ਸ਼ੁਰੂ_off,
						 box,
						 unit_len_ctx,
						 selstr,
						 whitespace_ਪਾਠ,
						 whitespace_ਲੰਮਾਈ);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}
			ਜੇ (before) {
				*ਪਹਿਲਾ = false;
				*before = WHITESPACE_NONE;
			}
		}
	}

	/* find the first child that could lie partially within the selection;
	 * this is important at the top-levels of the tree for pruning subtrees
	 * that lie entirely before the selection */

	ਜੇ (ਬੱਚਾ) {
		ਬਣਤਰ box *ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;

		ਜਦੋਂ (ਅਗਲਾ && ਅਗਲਾ->byte_ਵਿੱਥ < ਸ਼ੁਰੂ_idx) {
			ਬੱਚਾ = ਅਗਲਾ;
			ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;
		}

		ਜਦੋਂ (ਬੱਚਾ) {
			/* read before calling the handler in case it modifies
			 * the tree */
			ਬਣਤਰ box *ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;

			res = selection_copy(ਬੱਚਾ,
					     unit_len_ctx,
					     ਸ਼ੁਰੂ_idx,
					     ਸਿਰਾ_idx,
					     selstr,
					     before,
					     ਪਹਿਲਾ,
					     false);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}

			ਬੱਚਾ = ਅਗਲਾ;
		}
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Label each text box in the given box subtree with its position
 * in a textual representation of the content.
 *
 * \param box The box at root of subtree
 * \param idx current position within textual representation
 * \return updated position
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ selection_label_subtree(ਬਣਤਰ box *box, ਬਿਨਾਨਿਸ਼ਾਨੀ idx)
{
	ਬਣਤਰ box *ਬੱਚਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(box != ਨਿਰਾ);

	ਬੱਚਾ = box->children;

	box->byte_ਵਿੱਥ = idx;

	ਜੇ (box->ਪਾਠ) {
		idx += box->ਲੰਮਾਈ + SPACE_LEN(box);
	}

	ਜਦੋਂ (ਬੱਚਾ) {
		ਜੇ (ਬੱਚਾ->list_marker) {
			idx = selection_label_subtree(ਬੱਚਾ->list_marker, idx);
		}

		idx = selection_label_subtree(ਬੱਚਾ, idx);
		ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
	}

	ਵਾਪਸ idx;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_redraw(ਬਣਤਰ content *c,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx)
{
	nserror res;
	html_content *html = (html_content *)c;
	ਬਣਤਰ rdw_info rdw;

	ਜੇ (html->layout == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	rdw.inited = false;

	res = coords_from_range(html->layout, ਸ਼ੁਰੂ_idx, ਸਿਰਾ_idx, &rdw, false);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (rdw.inited) {
		content__request_redraw(c,
					rdw.r.x0,
					rdw.r.y0,
					rdw.r.x1 - rdw.r.x0,
					rdw.r.y1 - rdw.r.y0);
	}

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_copy(ਬਣਤਰ content *c,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
			ਬਣਤਰ selection_string *selstr)
{
	html_content *html = (html_content *)c;
	save_ਪਾਠ_whitespace before = WHITESPACE_NONE;
	bool ਪਹਿਲਾ = true;

	ਜੇ (html->layout == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਵਾਪਸ selection_copy(html->layout,
			      &html->unit_len_ctx,
			      ਸ਼ੁਰੂ_idx,
			      ਸਿਰਾ_idx,
			      selstr,
			      &before,
			      &ਪਹਿਲਾ,
			      false);
}


/* exported interface documented in html/textselection.h */
nserror
html_textselection_get_ਸਿਰਾ(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸਿਰਾ_idx)
{
	html_content *html = (html_content *)c;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਜੜ੍ਹ_idx;

	ਜੇ (html->layout == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਜੜ੍ਹ_idx = 0;

	*ਸਿਰਾ_idx = selection_label_subtree(html->layout, ਜੜ੍ਹ_idx);

	ਵਾਪਸ NSERROR_OK;
}
