/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
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
 * implementation of user interaction with a CONTENT_HTML.
 */

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "desktop/browser_history.h"
#ಅನ್ತರ್ಭಾವ "desktop/frames.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"
#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_textarea.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/imagemap.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ಸ್ಥಿರ browser_pointer_shape get_pointer_shape(ಸಂರಚನಾ box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	ಗಣನಾ css_cursor_e cursor;
	lwc_string **cursor_uris;

	ಯದಿ (box->type == BOX_FLOAT_ಎಡ || box->type == BOX_FLOAT_ಬಲ)
		style = box->children->style;
	ಅನ್ಯಥಾ
		style = box->style;

	ಯದಿ (style == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ವಿಕಲ್ಪ (cursor) {
	ಪಕ್ಷ CSS_CURSOR_AUTO:
		ಯದಿ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} ಅನ್ಯಥಾ ಯದಿ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} ಅನ್ಯಥಾ {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_ಬಲ;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_ಎಡ;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		pointer = BROWSER_POINTER_DEFAULT;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_box_drag_start(ಸಂರಚನಾ box *box, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಪೂರ್ಣಾಙ್ಕ box_x, box_y;
	ಪೂರ್ಣಾಙ್ಕ scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ಯದಿ (box->scroll_x != ರಿಕ್ತ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} ಅನ್ಯಥಾ ಯದಿ (box->scroll_y != ರಿಕ್ತ) {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				box->width + box->padding[ಬಲ_2] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_start_content_drag(box->scroll_y,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param html   html content
 * \param mouse  state of mouse buttons and modifier keys
 * \param x	 coordinate of mouse
 * \param y	 coordinate of mouse
 * \param dir    Direction of drag
 */
ಸ್ಥಿರ size_t html_selection_drag_end(ಸಂರಚನಾ html_content *html,
		browser_mouse_state mouse, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ dir)
{
	ಪೂರ್ಣಾಙ್ಕ pixel_offset;
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ಯದಿ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	ಪ್ರತ್ಯಾಗಮ idx;
}


/**
 * Helper for file gadgets to store their filename.
 *
 * Stores the filename unencoded on the dom node associated with the
 * gadget.
 *
 * \todo Get rid of this crap eventually
 *
 * \param operation DOM operation
 * \param key DOM node key being considerd
 * \param _data The data assocated with the key
 * \param src The source DOM node.
 * \param dst The destination DOM node.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
html__image_coords_dom_user_data_handler(dom_ಗಂಟು_operation operation,
					 dom_string *ಕೀಲಿ,
					 ಶೂನ್ಯ *_data,
					 ಸಂರಚನಾ dom_ಗಂಟು *src,
					 ಸಂರಚನಾ dom_ಗಂಟು *dst)
{
	ಸಂರಚನಾ image_ಒಳಹರಿವು_coords *oldcoords, *coords = _data, *newcoords;

	ಯದಿ (!dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_data,
				ಕೀಲಿ) || coords == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ (operation) {
	ಪಕ್ಷ DOM_ಗಂಟು_CLONED:
		newcoords = calloc(1, ಪರಿಮಾಣ(*newcoords));
		ಯದಿ (newcoords != ರಿಕ್ತ) {
			*newcoords = *coords;
			ಯದಿ (dom_ಗಂಟು_set_user_data(dst,
				 corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				ಮುಕ್ತ(oldcoords);
			}
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_DELETED:
		ಮುಕ್ತ(coords);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_RENAMED:
	ಪಕ್ಷ DOM_ಗಂಟು_IMPORTED:
	ಪಕ್ಷ DOM_ಗಂಟು_ADOPTED:
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param  scrollbar  scrollbar widget
 * \param  mouse   state of mouse buttons and modifier keys
 * \param  x	   coordinate of mouse
 * \param  y	   coordinate of mouse
 */
ಸ್ಥಿರ ಶೂನ್ಯ
html_overflow_scroll_drag_end(ಸಂರಚನಾ scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಪೂರ್ಣಾಙ್ಕ scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ಸಂರಚನಾ html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	ಸಂರಚನಾ box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	ಯದಿ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} ಅನ್ಯಥಾ {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				box->width + box->padding[ಬಲ_2] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ಸ್ಥಿರ nserror
mouse_action_select_menu(html_content *html,
			 ಸಂರಚನಾ browser_window *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0;
	ಪೂರ್ಣಾಙ್ಕ box_y = 0;
	ಧ್ರುವ ವರ್ಣ *status;
	ಪೂರ್ಣಾಙ್ಕ width, height;
	ಸಂರಚನಾ hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ಪ್ರತಿಜ್ಞಾ(html->visible_select_menu != ರಿಕ್ತ);

	bw_drag_type = browser_window_get_drag_type(bw);
	ಯದಿ (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ಎಡ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[ತುದಿ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ಯದಿ (status != ರಿಕ್ತ) {
		/* set status if menu still open */
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
		ಸಂದೇಶ_data.explicit_status_text = status;
		content_broadcast((ಸಂರಚನಾ content *)html,
				  CONTENT_ಸಂದೇಶ_STATUS,
				  &ಸಂದೇಶ_data);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = ರಿಕ್ತ;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ಸ್ಥಿರ nserror
mouse_action_drag_selection(html_content *html,
			    ಸಂರಚನಾ browser_window *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ dir = -1;
	ಪೂರ್ಣಾಙ್ಕ dx, dy;
	size_t idx;
	ಸಙ್ಘ html_drag_owner drag_owner;
	ಪೂರ್ಣಾಙ್ಕ pixel_offset;
	plot_font_style_t fstyle;

	ಯದಿ (!mouse) {
		/* End of selection drag */
		ಯದಿ (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		ಯದಿ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, ರಿಕ್ತ);

		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	ಯದಿ (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ಯದಿ (box != ರಿಕ್ತ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ಸ್ಥಿರ nserror
mouse_action_drag_scrollbar(html_content *html,
			    ಸಂರಚನಾ browser_window *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ scrollbar *scr;
	ಸಂರಚನಾ html_scrollbar_data *data;
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0;
	ಪೂರ್ಣಾಙ್ಕ box_y = 0;
	ಧ್ರುವ ವರ್ಣ *status;
	ಪೂರ್ಣಾಙ್ಕ scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	ಯದಿ (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	ಯದಿ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} ಅನ್ಯಥಾ {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				      box->width + box->padding[ಬಲ_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	ಯದಿ (status != ರಿಕ್ತ) {
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

		ಸಂದೇಶ_data.explicit_status_text = status;
		content_broadcast((ಸಂರಚನಾ content *)html,
				  CONTENT_ಸಂದೇಶ_STATUS,
				  &ಸಂದೇಶ_data);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ಸ್ಥಿರ nserror
mouse_action_drag_textarea(html_content *html,
			    ಸಂರಚನಾ browser_window *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0;
	ಪೂರ್ಣಾಙ್ಕ box_y = 0;

	box = html->drag_owner.textarea;

	ಪ್ರತಿಜ್ಞಾ(box->gadget != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ಸ್ಥಿರ nserror
mouse_action_drag_content(html_content *html,
			  ಸಂರಚನಾ browser_window *bw,
			  browser_mouse_state mouse,
			  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0;
	ಪೂರ್ಣಾಙ್ಕ box_y = 0;

	box = html->drag_owner.content;
	ಪ್ರತಿಜ್ಞಾ(box->object != ರಿಕ್ತ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ಸಂರಚನಾ mouse_action_state {
	ಸಂರಚನಾ {
		ಧ್ರುವ ವರ್ಣ *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		ಗಣನಾ {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_BACK, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} result;

	/** dom node */
	ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು;

	/** html object */
	ಸಂರಚನಾ {
		ಸಂರಚನಾ box *box;
		ಪೂರ್ಣಾಙ್ಕ pos_x;
		ಪೂರ್ಣಾಙ್ಕ pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ಸಂರಚನಾ browser_window *iframe;

	/** link either from href or imagemap */
	ಸಂರಚನಾ {
		ಸಂರಚನಾ box *box;
		nsurl *url;
		ಧ್ರುವ ವರ್ಣ *target;
		bool is_imagemap;
	} link;

	/** gadget */
	ಸಂರಚನಾ {
		ಸಂರಚನಾ form_control *control;
		ಸಂರಚನಾ box *box;
		ಪೂರ್ಣಾಙ್ಕ box_x;
		ಪೂರ್ಣಾಙ್ಕ box_y;
		ಧ್ರುವ ವರ್ಣ *target;
	} gadget;

	/** title */
	ಧ್ರುವ ವರ್ಣ *title;

	/** candidate box for drag operation */
	ಸಂರಚನಾ box *drag_candidate;

	/** scrollbar */
	ಸಂರಚನಾ {
		ಸಂರಚನಾ scrollbar *bar;
		ಪೂರ್ಣಾಙ್ಕ mouse_x;
		ಪೂರ್ಣಾಙ್ಕ mouse_y;
	} scroll;

	/** text in box */
	ಸಂರಚನಾ {
		ಸಂರಚನಾ box *box;
		ಪೂರ್ಣಾಙ್ಕ box_x;
	} text;
};


/**
 * iterate the box tree for deepest node at coordinates
 *
 * extracts mouse action node information by descending through
 *  visible boxes setting more specific values for:
 *
 * box - deepest box at point
 * html_object_box - html object
 * html_object_pos_x - html object
 * html_object_pos_y - html object
 * object - non html object
 * iframe - iframe
 * url - href or imagemap
 * target - href or imagemap or gadget
 * url_box - href or imagemap
 * imagemap - imagemap
 * gadget - gadget
 * gadget_box - gadget
 * gadget_box_x - gadget
 * gadget_box_y - gadget
 * title - title
 * pointer
 *
 * drag_candidate - first box with scroll
 * padding_left - box with scroll
 * padding_right
 * padding_top
 * padding_bottom
 * scrollbar - inside padding box stops decent
 * scroll_mouse_x - inside padding box stops decent
 * scroll_mouse_y - inside padding box stops decent
 *
 * text_box - text box
 * text_box_x - text_box
 */
ಸ್ಥಿರ nserror
get_mouse_action_ಗಂಟು(html_content *html,
		      ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		      ಸಂರಚನಾ mouse_action_state *man)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0;
	ಪೂರ್ಣಾಙ್ಕ box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ mouse_action_state));
	man->ಗಂಟು = html->layout->ಗಂಟು; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ಎಡ_2];
	box_y = box->margin[ತುದಿ_2];

	ಕುರು {
		/* skip hidden boxes */
		ಯದಿ ((box->style != ರಿಕ್ತ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			ಗಚ್ಛ next_box;
		}

		ಯದಿ (box->ಗಂಟು != ರಿಕ್ತ) {
			man->ಗಂಟು = box->ಗಂಟು;
		}

		ಯದಿ (box->object) {
			ಯದಿ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} ಅನ್ಯಥಾ {
				man->object = box->object;
			}
		}

		ಯದಿ (box->iframe) {
			man->iframe = box->iframe;
		}

		ಯದಿ (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ಯದಿ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ಯದಿ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ಯದಿ (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		ಯದಿ (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		ಯದಿ ((box->scroll_x != ರಿಕ್ತ) ||
		    (box->scroll_y != ರಿಕ್ತ)) {
			ಪೂರ್ಣಾಙ್ಕ padding_ಎಡ;
			ಪೂರ್ಣಾಙ್ಕ padding_ಬಲ;
			ಪೂರ್ಣಾಙ್ಕ padding_ತುದಿ;
			ಪೂರ್ಣಾಙ್ಕ padding_bottom;

			ಯದಿ (man->drag_candidate == ರಿಕ್ತ) {
				man->drag_candidate = box;
			}

			padding_ಎಡ = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_ಬಲ = padding_ಎಡ + box->padding[ಎಡ_2] +
					box->width + box->padding[ಬಲ_2];
			padding_ತುದಿ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_ತುದಿ + box->padding[ತುದಿ_2] +
					box->height + box->padding[BOTTOM];

			ಯದಿ ((x > padding_ಎಡ) &&
			    (x < padding_ಬಲ) &&
			    (y > padding_ತುದಿ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ಯದಿ ((box->scroll_y != ರಿಕ್ತ) &&
				    (x > (padding_ಬಲ - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ಬಲ - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_ತುದಿ;
					ಭಙ್ಗ;

				} ಅನ್ಯಥಾ ಯದಿ ((box->scroll_x != ರಿಕ್ತ) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ಎಡ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					ಭಙ್ಗ;
				}
			}
		}

		ಯದಿ (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ಯಾವತ್ (box != ರಿಕ್ತ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ಪ್ರತಿಜ್ಞಾ(man->ಗಂಟು != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ಸ್ಥಿರ nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		    ಸಂರಚನಾ mouse_action_state *mas)
{
	ಸಂರಚನಾ content *c = (ಸಂರಚನಾ content *)html;
	textarea_mouse_status ta_status;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ವಿಕಲ್ಪ (mas->gadget.control->type) {
	ಪಕ್ಷ GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ಯದಿ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ರಿಕ್ತ;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
			ಸಂದೇಶ_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_ಸಂದೇಶ_SELECTMENU,
					  &ಸಂದೇಶ_data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_ಒಳಹರಿವು_element_set_checked(
				(dom_html_ಒಳಹರಿವು_element *)(mas->gadget.control->ಗಂಟು),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
			ಸಂರಚನಾ image_ಒಳಹರಿವು_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, ಪರಿಮಾಣ(*coords));
			ಯದಿ (coords == ರಿಕ್ತ) {
				ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ಯದಿ (dom_ಗಂಟು_set_user_data(
				mas->gadget.control->ಗಂಟು,
				corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
			}
			ಮುಕ್ತ(oldcoords);
		}
		fallthrough;

	ಪಕ್ಷ GADGET_SUBMIT:
		ಯದಿ (mas->gadget.control->form) {
			ಸ್ಥಿರ ವರ್ಣ status_buffer[200];

			snprintf(status_buffer,
				 ಪರಿಮಾಣ status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			ಯದಿ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} ಅನ್ಯಥಾ {
			mas->result.status = messages_get("FormBadSubmit");
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_TEXTBOX:
	ಪಕ್ಷ GADGET_PASSWORD:
	ಪಕ್ಷ GADGET_TEXTAREA:
		ಯದಿ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} ಅನ್ಯಥಾ {
			mas->result.status = messages_get("FormTextbox");
		}

		ಯದಿ (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			ಸಙ್ಘ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_set_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_status = textarea_mouse_action(mas->gadget.control->data.text.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		ಯದಿ (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} ಅನ್ಯಥಾ {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_HIDDEN:
		/* not possible: no box generated */
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
			ಸಂದೇಶ_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_ಸಂದೇಶ_GADGETCLICK,
					  &ಸಂದೇಶ_data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ಸ್ಥಿರ nserror
iframe_mouse_action(ಸಂರಚನಾ browser_window *bw,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		    ಸಂರಚನಾ mouse_action_state *mas)
{
	ಪೂರ್ಣಾಙ್ಕ pos_x, pos_y;
	ಭಿನ್ನ scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} ಅನ್ಯಥಾ {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ಸ್ಥಿರ nserror
html_object_mouse_action(html_content *html,
			 ಸಂರಚನಾ browser_window *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			 ಸಂರಚನಾ mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ಯದಿ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		ಸಙ್ಘ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} ಅನ್ಯಥಾ {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ಸ್ಥಿರ bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಯದಿ (scheme != ರಿಕ್ತ) {
		ಯದಿ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ಪ್ರತ್ಯಾಗಮ is_js;
}


/**
 * process mouse activity on a link
 */
ಸ್ಥಿರ nserror
link_mouse_action(html_content *html,
		  ಸಂರಚನಾ browser_window *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		  ಸಂರಚನಾ mouse_action_state *mas)
{
	nserror res;
	ವರ್ಣ *url_s = ರಿಕ್ತ;
	size_t url_l = 0;
	ಸ್ಥಿರ ವರ್ಣ status_buffer[200];
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

	ಯದಿ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ಯದಿ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ರಿಕ್ತ;
		}
	}

	ಯದಿ (mas->title) {
		snprintf(status_buffer,
			 ಪರಿಮಾಣ status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} ಅನ್ಯಥಾ {
		snprintf(status_buffer,
			 ಪರಿಮಾಣ status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ಯದಿ (url_s != ರಿಕ್ತ) {
		ಮುಕ್ತ(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((ಸಂರಚನಾ content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ರಿಕ್ತ,
					ರಿಕ್ತ,
					ರಿಕ್ತ);

	} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		ಸಂದೇಶ_data.savelink.url = mas->link.url;
		ಸಂದೇಶ_data.savelink.title = mas->title;
		content_broadcast((ಸಂರಚನಾ content *)html,
				  CONTENT_ಸಂದೇಶ_SAVELINK,
				  &ಸಂದೇಶ_data);

	} ಅನ್ಯಥಾ ಯದಿ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ಯದಿ (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} ಅನ್ಯಥಾ {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ಯದಿ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		ಸಙ್ಘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ರಿಕ್ತ);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ಸ್ಥಿರ nserror
default_mouse_action(html_content *html,
		  ಸಂರಚನಾ browser_window *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		  ಸಂರಚನಾ mouse_action_state *mas)
{
	ಸಂರಚನಾ content *c = (ಸಂರಚನಾ content *)html;

	/* frame resizing */
	ಯದಿ (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		ಯದಿ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		ಪ್ರತ್ಯಾಗಮ default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	ಸಙ್ಘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ಯದಿ (click && html->focus_type != HTML_FOCUS_SELF) {
		ಸಙ್ಘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ರಿಕ್ತ);
	}
	ಯದಿ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಯದಿ (mas->text.box) {
		ಪೂರ್ಣಾಙ್ಕ pixel_offset;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->text.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->text.box->text,
				       mas->text.box->length,
				       x - mas->text.box_x,
				       &idx,
				       &pixel_offset);

		ಯದಿ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			ಸಙ್ಘ html_drag_owner drag_owner;

			ಯದಿ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   ರಿಕ್ತ);
				mas->result.status = messages_get("Selecting");
			}

			ಯದಿ (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} ಅನ್ಯಥಾ ಯದಿ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ಪ್ರತ್ಯಾಗಮ default_mouse_action_focus(html, mouse);
		}

	} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ಯದಿ (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} ಅನ್ಯಥಾ ಯದಿ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಯದಿ (mas->title) {
		mas->result.status = mas->title;
	}

	ಯದಿ (mouse & BROWSER_MOUSE_DRAG_1) {
		ಯದಿ (mouse & BROWSER_MOUSE_MOD_2) {
			ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
			ಸಂದೇಶ_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			ಸಂದೇಶ_data.dragsave.content = ರಿಕ್ತ;
			content_broadcast(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_data);
		} ಅನ್ಯಥಾ {
			ಯದಿ (mas->drag_candidate == ರಿಕ್ತ) {
				browser_window_page_drag_start(bw, x, y);
			} ಅನ್ಯಥಾ {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_DRAG_2) {
		ಯದಿ (mouse & BROWSER_MOUSE_MOD_2) {
			ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
			ಸಂದೇಶ_data.dragsave.type = CONTENT_SAVE_SOURCE;
			ಸಂದೇಶ_data.dragsave.content = ರಿಕ್ತ;
			content_broadcast(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_data);
		} ಅನ್ಯಥಾ {
			ಯದಿ (mas->drag_candidate == ರಿಕ್ತ) {
				browser_window_page_drag_start(bw, x, y);
			} ಅನ್ಯಥಾ {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	ಪ್ರತ್ಯಾಗಮ default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ಸ್ಥಿರ nserror
mouse_action_drag_none(html_content *html,
		       ಸಂರಚನಾ browser_window *bw,
		       browser_mouse_state mouse,
		       ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	nserror res;
	ಸಂರಚನಾ content *c = (ಸಂರಚನಾ content *)html;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ಸ್ಥಿರ ಸಂರಚನಾ mouse_action_state mas;

	res = get_mouse_action_ಗಂಟು(html, x, y, &mas);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಯದಿ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} ಅನ್ಯಥಾ ಯದಿ (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} ಅನ್ಯಥಾ ಯದಿ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} ಅನ್ಯಥಾ ಯದಿ ((mas.object != ರಿಕ್ತ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ಯದಿ (mouse & BROWSER_MOUSE_DRAG_2) {
			ಸಂದೇಶ_data.dragsave.type = CONTENT_SAVE_NATIVE;
			ಸಂದೇಶ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_data);

		} ಅನ್ಯಥಾ ಯದಿ (mouse & BROWSER_MOUSE_DRAG_1) {
			ಸಂದೇಶ_data.dragsave.type = CONTENT_SAVE_ORIG;
			ಸಂದೇಶ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_data);
		}

		/* \todo should have a drag-saving object msg */

	} ಅನ್ಯಥಾ ಯದಿ (mas.iframe != ರಿಕ್ತ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} ಅನ್ಯಥಾ ಯದಿ (mas.html_object.box != ರಿಕ್ತ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} ಅನ್ಯಥಾ ಯದಿ (mas.link.url != ರಿಕ್ತ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} ಅನ್ಯಥಾ {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* send status and pointer message */
	ಯದಿ (mas.result.action != ACTION_NOSEND) {
		ಸಂದೇಶ_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_STATUS, &ಸಂದೇಶ_data);

		ಸಂದೇಶ_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_POINTER, &ಸಂದೇಶ_data);
	}

	/* fire dom click event */
	ಯದಿ (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ಗಂಟು, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ವಿಕಲ್ಪ (mas.result.action) {
	ಪಕ್ಷ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ಭಙ್ಗ;

	ಪಕ್ಷ ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ರಿಕ್ತ,
				ರಿಕ್ತ,
				ರಿಕ್ತ);
		ಭಙ್ಗ;

	ಪಕ್ಷ ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		ಯದಿ (path != ರಿಕ್ತ) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ ACTION_BACK:
		res = browser_window_history_back(bw, false);
		ಭಙ್ಗ;

	ಪಕ್ಷ ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		ಭಙ್ಗ;

	ಪಕ್ಷ ACTION_NOSEND:
	ಪಕ್ಷ ACTION_NONE:
		res = NSERROR_OK;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ಸಂರಚನಾ content *c,
			 ಸಂರಚನಾ browser_window *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಪ್ರತ್ಯಾಗಮ html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ಸಂರಚನಾ content *c,
		  ಸಂರಚನಾ browser_window *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ಯದಿ (html->visible_select_menu != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ವಿಕಲ್ಪ (html->drag_type) {
	ಪಕ್ಷ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_TEXTAREA_SELECTION:
	ಪಕ್ಷ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_CONTENT_SELECTION:
	ಪಕ್ಷ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		/* Unknown content related drag type */
		ಪ್ರತಿಜ್ಞಾ(0 && "Unknown content related drag type");
	}

	ಯದಿ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ಪ್ರತ್ಯಾಗಮ res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ಸಂರಚನಾ content *c, uint32_t ಕೀಲಿ)
{
	html_content *html = (html_content *) c;
	ಸಂರಚನಾ selection *sel = html->sel;

	/** \todo
	 * At the moment, the front end interface for keypress only gives
	 * us a UCS4 key value.  This doesn't doesn't have all the information
	 * we need to fill out the event properly.  We don't get to know about
	 * modifier keys, and things like CTRL+C are passed in as
	 * \ref NS_KEY_COPY_SELECTION, a magic value outside the valid Unicode
	 * range.
	 *
	 * We need to:
	 *
	 * 1. Update the front end interface so that both press and release
	 *    events reach the core.
	 * 2. Stop encoding the special keys like \ref NS_KEY_COPY_SELECTION as
	 *    magic values in the front ends, so we just get the events, e.g.:
	 *    1. Press ctrl
	 *    2. Press c
	 *    3. Release c
	 *    4. Release ctrl
	 * 3. Pass all the new info to the DOM KeyboardEvent events.
	 * 4. If there is a focused element, fire the event at that, instead of
	 *    `html->layout->node`.
	 * 5. Rebuild the \ref NS_KEY_COPY_SELECTION values from the info we
	 *    now get given, and use that for the code below this
	 *    \ref fire_dom_keyboard_event call.
	 * 6. Move the code after this \ref fire_dom_keyboard_event call into
	 *    the default action handler for DOM events.
	 *
	 * This will mean that if the JavaScript event listener does
	 * `event.preventDefault()` then we won't handle the event when
	 * we're not supposed to.
	 */
	ಯದಿ (html->layout != ರಿಕ್ತ && html->layout->ಗಂಟು != ರಿಕ್ತ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ಗಂಟು, true, true, ಕೀಲಿ);
	}

	ವಿಕಲ್ಪ (html->focus_type) {
	ಪಕ್ಷ HTML_FOCUS_CONTENT:
		ಪ್ರತ್ಯಾಗಮ content_keypress(html->focus_owner.content->object, ಕೀಲಿ);

	ಪಕ್ಷ HTML_FOCUS_TEXTAREA:
		ಯದಿ (box_textarea_keypress(html, html->focus_owner.textarea, ಕೀಲಿ) == NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ true;
		} ಅನ್ಯಥಾ {
			ಪ್ರತ್ಯಾಗಮ false;
		}

	ಸ್ವಭಾವ:
		/* Deal with it below */
		ಭಙ್ಗ;
	}

	ವಿಕಲ್ಪ (ಕೀಲಿ) {
	ಪಕ್ಷ NS_ಕೀಲಿ_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_SELECT_ALL:
		selection_select_all(sel);
		ಪ್ರತ್ಯಾಗಮ true;

	ಪಕ್ಷ NS_ಕೀಲಿ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ಪ್ರತ್ಯಾಗಮ selection_clear(sel, true);
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Callback for in-page scrollbars.
 */
ಶೂನ್ಯ html_overflow_scroll_callback(ಶೂನ್ಯ *client_data,
		ಸಂರಚನಾ scrollbar_ಸಂದೇಶ_data *scrollbar_data)
{
	ಸಂರಚನಾ html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	ಸಂರಚನಾ box *box = data->box;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	html_drag_type drag_type;
	ಸಙ್ಘ html_drag_owner drag_owner;

	ವಿಕಲ್ಪ(scrollbar_data->ಸಂದೇಶ) {
	ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_MOVED:

		ಯದಿ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ಭಙ್ಗ;
		}

		html__redraw_a_box(html, box);
		ಭಙ್ಗ;
	ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_SCROLL_START:
	{
		ಸಂರಚನಾ rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		ಭಙ್ಗ;
	ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, ರಿಕ್ತ);

		ಸಂದೇಶ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_ಸಂದೇಶ_POINTER, &ಸಂದೇಶ_data);
		ಭಙ್ಗ;
	}
}


/* Documented in html_internal.h */
ಶೂನ್ಯ html_set_drag_type(html_content *html, html_drag_type drag_type,
		ಸಙ್ಘ html_drag_owner drag_owner, ಧ್ರುವ ಸಂರಚನಾ rect *rect)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

	ಪ್ರತಿಜ್ಞಾ(html != ರಿಕ್ತ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ವಿಕಲ್ಪ (drag_type) {
	ಪಕ್ಷ HTML_DRAG_NONE:
		ಪ್ರತಿಜ್ಞಾ(drag_owner.no_owner == true);
		ಸಂದೇಶ_data.drag.type = CONTENT_DRAG_NONE;
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_SCROLLBAR:
	ಪಕ್ಷ HTML_DRAG_TEXTAREA_SCROLLBAR:
	ಪಕ್ಷ HTML_DRAG_CONTENT_SCROLL:
		ಸಂದೇಶ_data.drag.type = CONTENT_DRAG_SCROLL;
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_DRAG_SELECTION:
		ಪ್ರತಿಜ್ಞಾ(drag_owner.no_owner == true);
		fallthrough;
	ಪಕ್ಷ HTML_DRAG_TEXTAREA_SELECTION:
	ಪಕ್ಷ HTML_DRAG_CONTENT_SELECTION:
		ಸಂದೇಶ_data.drag.type = CONTENT_DRAG_SELECTION;
		ಭಙ್ಗ;
	}
	ಸಂದೇಶ_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((ಸಂರಚನಾ content *)html, CONTENT_ಸಂದೇಶ_DRAG, &ಸಂದೇಶ_data);
}

/* Documented in html_internal.h */
ಶೂನ್ಯ html_set_focus(html_content *html, html_focus_type focus_type,
		ಸಙ್ಘ html_focus_owner focus_owner, bool hide_caret,
		ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ height, ಧ್ರುವ ಸಂರಚನಾ rect *clip)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಪೂರ್ಣಾಙ್ಕ x_off = 0;
	ಪೂರ್ಣಾಙ್ಕ y_off = 0;
	ಸಂರಚನಾ rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ಪ್ರತಿಜ್ಞಾ(html != ರಿಕ್ತ);

	ವಿಕಲ್ಪ (focus_type) {
	ಪಕ್ಷ HTML_FOCUS_SELF:
		ಪ್ರತಿಜ್ಞಾ(focus_owner.self == true);
		ಯದಿ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ಪ್ರತ್ಯಾಗಮ;
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ಭಙ್ಗ;

	ಪಕ್ಷ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ಭಙ್ಗ;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ಯದಿ (textarea_lost_focus) {
		ಸಂದೇಶ_data.caret.type = CONTENT_CARET_REMOVE;
	} ಅನ್ಯಥಾ ಯದಿ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		ಸಂದೇಶ_data.caret.type = CONTENT_CARET_HIDE;
	} ಅನ್ಯಥಾ {
		ಯದಿ (clip != ರಿಕ್ತ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		ಸಂದೇಶ_data.caret.type = CONTENT_CARET_SET_POS;
		ಸಂದೇಶ_data.caret.pos.x = x + x_off;
		ಸಂದೇಶ_data.caret.pos.y = y + y_off;
		ಸಂದೇಶ_data.caret.pos.height = height;
		ಸಂದೇಶ_data.caret.pos.clip = (clip == ರಿಕ್ತ) ? ರಿಕ್ತ : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((ಸಂರಚನಾ content *)html, CONTENT_ಸಂದೇಶ_CARET, &ಸಂದೇಶ_data);
}

/* Documented in html_internal.h */
ಶೂನ್ಯ html_set_selection(html_content *html, html_selection_type selection_type,
		ಸಙ್ಘ html_selection_owner selection_owner, bool read_only)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಸಂರಚನಾ box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	ಪ್ರತಿಜ್ಞಾ(html != ರಿಕ್ತ);

	ಯದಿ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	ಯದಿ (html->selection_type != HTML_SELECTION_NONE) {
		ವಿಕಲ್ಪ (html->selection_type) {
		ಪಕ್ಷ HTML_SELECTION_SELF:
			ಯದಿ (same_type)
				ಭಙ್ಗ;
			selection_clear(html->sel, true);
			ಭಙ್ಗ;
		ಪಕ್ಷ HTML_SELECTION_TEXTAREA:
			ಯದಿ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ಭಙ್ಗ;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			ಭಙ್ಗ;
		ಪಕ್ಷ HTML_SELECTION_CONTENT:
			ಯದಿ (same_type && html->selection_owner.content ==
					selection_owner.content)
				ಭಙ್ಗ;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ಯದಿ (!changed)
		/* Don't need to report lack of change to owner */
		ಪ್ರತ್ಯಾಗಮ;

	/* Prepare msg */
	ವಿಕಲ್ಪ (selection_type) {
	ಪಕ್ಷ HTML_SELECTION_NONE:
		ಪ್ರತಿಜ್ಞಾ(selection_owner.none == true);
		ಸಂದೇಶ_data.selection.selection = false;
		ಭಙ್ಗ;
	ಪಕ್ಷ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(selection_owner.none == false);
		fallthrough;
	ಪಕ್ಷ HTML_SELECTION_TEXTAREA:
	ಪಕ್ಷ HTML_SELECTION_CONTENT:
		ಸಂದೇಶ_data.selection.selection = true;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}
	ಸಂದೇಶ_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((ಸಂರಚನಾ content *)html, CONTENT_ಸಂದೇಶ_SELECTION,
			&ಸಂದೇಶ_data);
}
