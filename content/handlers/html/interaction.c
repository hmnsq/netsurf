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

ಸ್ಥಿರ browser_ಸೂಚಕ_shape get_ಸೂಚಕ_shape(ರಚನೆ box *box, bool imagemap)
{
	browser_ಸೂಚಕ_shape ಸೂಚಕ;
	css_computed_style *style;
	ಎಣಿಕೆ css_cursor_e cursor;
	lwc_string **cursor_uris;

	ಆದರೆ (box->type == BOX_FLOAT_ಎಡ || box->type == BOX_FLOAT_ಬಲ)
		style = box->children->style;
	ಇಲ್ಲದಿದ್ದರೆ
		style = box->style;

	ಆದರೆ (style == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ BROWSER_ಸೂಚಕ_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ಆಯ್ಕೆ (cursor) {
	ಸ್ಥಿತಿ CSS_CURSOR_AUTO:
		ಆದರೆ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಬಿಂದು;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			ಸೂಚಕ = BROWSER_ಸೂಚಕ_CARET;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* html content doesn't mind */
			ಸೂಚಕ = BROWSER_ಸೂಚಕ_AUTO;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_CROSSHAIR:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_CROSS;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಸೂಚಕ:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಬಿಂದು;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_MOVE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_MOVE;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_E_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಬಲ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_W_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಎಡ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_N_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಮೇಲೆ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_S_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಕೆಳಕ್ಕೆ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_NE_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_RU;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_SW_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_LD;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_SE_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_RD;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_NW_RESIZE:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_LU;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಪಠ್ಯ:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_CARET;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_WAIT:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_WAIT;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_ಪ್ರಗತಿ:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_ಪ್ರಗತಿ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_CURSOR_HELP:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_HELP;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಸೂಚಕ;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_box_drag_ಆರಂಭ(ರಚನೆ box *box, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ಪೂರ್ಣಾಂಕ box_x, box_y;
	ಪೂರ್ಣಾಂಕ scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ಆದರೆ (box->scroll_x != ಖಾಲಿ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				box->ಎತ್ತರ + box->padding[BOTTOM] -
				SCROLLBAR_ಅಗಲ);
		scrollbar_ಆರಂಭ_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->scroll_y != ಖಾಲಿ) {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				box->ಅಗಲ + box->padding[ಬಲ_2] -
				SCROLLBAR_ಅಗಲ);
		scroll_mouse_y = y - box_y;

		scrollbar_ಆರಂಭ_content_drag(box->scroll_y,
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
ಸ್ಥಿರ size_t html_selection_drag_ಕೊನೆ(ರಚನೆ html_content *html,
		browser_mouse_state mouse, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ dir)
{
	ಪೂರ್ಣಾಂಕ pixel_ಸ್ಥಾನಾಂತರ;
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ dx, dy;
	size_t idx = 0;

	box = box_pick_ಪಠ್ಯ_box(html, x, y, dir, &dx, &dy);
	ಆದರೆ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->ಪಠ್ಯ, box->ಉದ್ದಳತೆ,
				       dx, &idx, &pixel_ಸ್ಥಾನಾಂತರ);

		idx += box->byte_ಸ್ಥಾನಾಂತರ;
	}

	ಹಿಂತಿರುಗಿ idx;
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
html_image_coords_dom_user_ದತ್ತಾಂಶ_handler(dom_ಗಂಟು_operation operation,
					 dom_string *ಕೀಲಿ,
					 ಶೂನ್ಯ *ದತ್ತಾಂಶ_2,
					 ರಚನೆ dom_ಗಂಟು *src,
					 ರಚನೆ dom_ಗಂಟು *dst)
{
	ರಚನೆ image_ಒಳಹರಿವು_coords *oldcoords, *coords = ದತ್ತಾಂಶ_2, *newcoords;

	ಆದರೆ (!dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_ದತ್ತಾಂಶ,
				ಕೀಲಿ) || coords == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ (operation) {
	ಸ್ಥಿತಿ DOM_ಗಂಟು_CLONED:
		newcoords = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*newcoords));
		ಆದರೆ (newcoords != ಖಾಲಿ) {
			*newcoords = *coords;
			ಆದರೆ (dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(dst,
				 corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_ದತ್ತಾಂಶ,
				 newcoords,
				 html_image_coords_dom_user_ದತ್ತಾಂಶ_handler,
				 &oldcoords) == DOM_NO_ERR) {
				ಬಿಡುಗಡೆ(oldcoords);
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_DELETED:
		ಬಿಡುಗಡೆ(coords);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_RENAMED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_IMPORTED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_ADOPTED:
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
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
html_ಅತಿಪ್ರವಾಹ_scroll_drag_ಕೊನೆ(ರಚನೆ scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ಪೂರ್ಣಾಂಕ scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(scrollbar);
	ರಚನೆ box *box;

	box = ದತ್ತಾಂಶ->box;
	box_coords(box, &box_x, &box_y);

	ಆದರೆ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				box->ಎತ್ತರ + box->padding[BOTTOM] -
				SCROLLBAR_ಅಗಲ);
		scrollbar_mouse_drag_ಕೊನೆ(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} ಇಲ್ಲದಿದ್ದರೆ {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				box->ಅಗಲ + box->padding[ಬಲ_2] -
				SCROLLBAR_ಅಗಲ);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_ಕೊನೆ(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ಸ್ಥಿರ nserror
mouse_action_select_menu(html_content *html,
			 ರಚನೆ browser_ಕಿಟಕಿ *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ box_x = 0;
	ಪೂರ್ಣಾಂಕ box_y = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
	ರಚನೆ hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ಪ್ರತಿಜ್ಞಾ(html->visible_select_menu != ಖಾಲಿ);

	bw_drag_type = browser_ಕಿಟಕಿ_get_drag_type(bw);
	ಆದರೆ (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_ಕೊನೆ(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ಎಡ_2].ಅಗಲ;
	box_y += box->ಎತ್ತರ + box->border[BOTTOM].ಅಗಲ +
		box->padding[BOTTOM] + box->padding[ತುದಿ_2];

	ಸ್ಥಿತಿ_2 = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ಆದರೆ (ಸ್ಥಿತಿ_2 != ಖಾಲಿ) {
		/* set status if menu still open */
		ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಸ್ಥಿತಿ_2;
		content_ಪ್ರಸಾರ((ರಚನೆ content *)html,
				  CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ,
				  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &ಅಗಲ, &ಎತ್ತರ);

	html->visible_select_menu = ಖಾಲಿ;

	bw_content = browser_ಕಿಟಕಿ_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       ಅಗಲ,
			       ಎತ್ತರ);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ಸ್ಥಿರ nserror
mouse_action_drag_selection(html_content *html,
			    ರಚನೆ browser_ಕಿಟಕಿ *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ dir = -1;
	ಪೂರ್ಣಾಂಕ dx, dy;
	size_t idx;
	ಒಕ್ಕೂಟ html_drag_owner drag_owner;
	ಪೂರ್ಣಾಂಕ pixel_ಸ್ಥಾನಾಂತರ;
	plot_font_style_t fstyle;

	ಆದರೆ (!mouse) {
		/* End of selection drag */
		ಆದರೆ (selection_dragging_ಆರಂಭ(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_ಕೊನೆ(html, mouse, x, y, dir);

		ಆದರೆ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_ಗಣ_drag_type(html, HTML_DRAG_NONE, drag_owner, ಖಾಲಿ);

		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (selection_dragging_ಆರಂಭ(html->sel)) {
		dir = 1;
	}

	box = box_pick_ಪಠ್ಯ_box(html, x, y, dir, &dx, &dy);
	ಆದರೆ (box != ಖಾಲಿ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->ಪಠ್ಯ,
				       box->ಉದ್ದಳತೆ,
				       dx,
				       &idx,
				       &pixel_ಸ್ಥಾನಾಂತರ);

		selection_track(html->sel, mouse, box->byte_ಸ್ಥಾನಾಂತರ + idx);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ಸ್ಥಿರ nserror
mouse_action_drag_scrollbar(html_content *html,
			    ರಚನೆ browser_ಕಿಟಕಿ *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ scrollbar *scr;
	ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ box_x = 0;
	ಪೂರ್ಣಾಂಕ box_y = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_ಸ್ಥಿತಿ scrollbar_ಸ್ಥಿತಿ;

	scr = html->drag_owner.scrollbar;

	ಆದರೆ (!mouse) {
		/* drag end: scrollbar */
		html_ಅತಿಪ್ರವಾಹ_scroll_drag_ಕೊನೆ(scr, mouse, x, y);
	}

	ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(scr);

	box = ದತ್ತಾಂಶ->box;

	box_coords(box, &box_x, &box_y);

	ಆದರೆ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ತುದಿ_2] +
				      box->ಎತ್ತರ + box->padding[BOTTOM] -
				      SCROLLBAR_ಅಗಲ);
		scrollbar_ಸ್ಥಿತಿ = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} ಇಲ್ಲದಿದ್ದರೆ {
		scroll_mouse_x = x - (box_x + box->padding[ಎಡ_2] +
				      box->ಅಗಲ + box->padding[ಬಲ_2] -
				      SCROLLBAR_ಅಗಲ);
		scroll_mouse_y = y - box_y;

		scrollbar_ಸ್ಥಿತಿ = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	ಸ್ಥಿತಿ_2 = scrollbar_mouse_ಸ್ಥಿತಿ_ಗಮ್ಯ_ಸಂದೇಶ(scrollbar_ಸ್ಥಿತಿ);

	ಆದರೆ (ಸ್ಥಿತಿ_2 != ಖಾಲಿ) {
		ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

		ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಸ್ಥಿತಿ_2;
		content_ಪ್ರಸಾರ((ರಚನೆ content *)html,
				  CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ,
				  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ಸ್ಥಿರ nserror
mouse_action_drag_textarea(html_content *html,
			    ರಚನೆ browser_ಕಿಟಕಿ *bw,
			    browser_mouse_state mouse,
			    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ box_x = 0;
	ಪೂರ್ಣಾಂಕ box_y = 0;

	box = html->drag_owner.textarea;

	ಪ್ರತಿಜ್ಞಾ(box->gadget != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ಸ್ಥಿರ nserror
mouse_action_drag_content(html_content *html,
			  ರಚನೆ browser_ಕಿಟಕಿ *bw,
			  browser_mouse_state mouse,
			  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ box_x = 0;
	ಪೂರ್ಣಾಂಕ box_y = 0;

	box = html->drag_owner.content;
	ಪ್ರತಿಜ್ಞಾ(box->object != ಖಾಲಿ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ರಚನೆ mouse_action_state {
	ರಚನೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_2; /**< status text */
		browser_ಸೂಚಕ_shape ಸೂಚಕ; /**< pointer shape */
		ಎಣಿಕೆ {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_ಹಿಂಭಾಗ, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} ಫಲಿತಾಂಶ;

	/** dom node */
	ರಚನೆ dom_ಗಂಟು *ಗಂಟು;

	/** html object */
	ರಚನೆ {
		ರಚನೆ box *box;
		ಪೂರ್ಣಾಂಕ pos_x;
		ಪೂರ್ಣಾಂಕ pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ರಚನೆ browser_ಕಿಟಕಿ *iframe;

	/** link either from href or imagemap */
	ರಚನೆ {
		ರಚನೆ box *box;
		nsurl *url;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ;
		bool is_imagemap;
	} link;

	/** gadget */
	ರಚನೆ {
		ರಚನೆ form_control *control;
		ರಚನೆ box *box;
		ಪೂರ್ಣಾಂಕ box_x;
		ಪೂರ್ಣಾಂಕ box_y;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ;
	} gadget;

	/** title */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;

	/** candidate box for drag operation */
	ರಚನೆ box *drag_candidate;

	/** scrollbar */
	ರಚನೆ {
		ರಚನೆ scrollbar *bar;
		ಪೂರ್ಣಾಂಕ mouse_x;
		ಪೂರ್ಣಾಂಕ mouse_y;
	} scroll;

	/** text in box */
	ರಚನೆ {
		ರಚನೆ box *box;
		ಪೂರ್ಣಾಂಕ box_x;
	} ಪಠ್ಯ;
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
		      ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		      ರಚನೆ mouse_action_state *man)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ box_x = 0;
	ಪೂರ್ಣಾಂಕ box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, ಗಾತ್ರ(ರಚನೆ mouse_action_state));
	man->ಗಂಟು = html->layout->ಗಂಟು; /* Default dom node to the <HTML> */
	man->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ಎಡ_2];
	box_y = box->margin[ತುದಿ_2];

	ಮಾಡು {
		/* skip hidden boxes */
		ಆದರೆ ((box->style != ಖಾಲಿ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			ಹೋಗು ಮುಂದಿನ_box;
		}

		ಆದರೆ (box->ಗಂಟು != ಖಾಲಿ) {
			man->ಗಂಟು = box->ಗಂಟು;
		}

		ಆದರೆ (box->object) {
			ಆದರೆ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} ಇಲ್ಲದಿದ್ದರೆ {
				man->object = box->object;
			}
		}

		ಆದರೆ (box->iframe) {
			man->iframe = box->iframe;
		}

		ಆದರೆ (box->href) {
			man->link.url = box->href;
			man->link.ಗುರಿ = box->ಗುರಿ;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ಆದರೆ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.ಗುರಿ);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ಆದರೆ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ಆದರೆ (box->gadget->form) {
				man->gadget.ಗುರಿ = box->gadget->form->ಗುರಿ;
			}
		}

		ಆದರೆ (box->title) {
			man->title = box->title;
		}

		man->ಫಲಿತಾಂಶ.ಸೂಚಕ = get_ಸೂಚಕ_shape(box, false);

		ಆದರೆ ((box->scroll_x != ಖಾಲಿ) ||
		    (box->scroll_y != ಖಾಲಿ)) {
			ಪೂರ್ಣಾಂಕ padding_ಎಡ;
			ಪೂರ್ಣಾಂಕ padding_ಬಲ;
			ಪೂರ್ಣಾಂಕ padding_ತುದಿ;
			ಪೂರ್ಣಾಂಕ padding_bottom;

			ಆದರೆ (man->drag_candidate == ಖಾಲಿ) {
				man->drag_candidate = box;
			}

			padding_ಎಡ = box_x +
					scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x);
			padding_ಬಲ = padding_ಎಡ + box->padding[ಎಡ_2] +
					box->ಅಗಲ + box->padding[ಬಲ_2];
			padding_ತುದಿ = box_y +
					scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y);
			padding_bottom = padding_ತುದಿ + box->padding[ತುದಿ_2] +
					box->ಎತ್ತರ + box->padding[BOTTOM];

			ಆದರೆ ((x > padding_ಎಡ) &&
			    (x < padding_ಬಲ) &&
			    (y > padding_ತುದಿ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ಆದರೆ ((box->scroll_y != ಖಾಲಿ) &&
				    (x > (padding_ಬಲ - SCROLLBAR_ಅಗಲ))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ಬಲ - SCROLLBAR_ಅಗಲ);
					man->scroll.mouse_y = y - padding_ತುದಿ;
					ನಿಲ್ಲಿಸು;

				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((box->scroll_x != ಖಾಲಿ) &&
					   (y > (padding_bottom -
							SCROLLBAR_ಅಗಲ))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ಎಡ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_ಅಗಲ);
					ನಿಲ್ಲಿಸು;
				}
			}
		}

		ಆದರೆ (box->ಪಠ್ಯ && !box->object) {
			man->ಪಠ್ಯ.box = box;
			man->ಪಠ್ಯ.box_x = box_x;
		}

	ಮುಂದಿನ_box:
		/* iterate to next box */
		box = box_at_ಬಿಂದು(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ಆಗ (box != ಖಾಲಿ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ಪ್ರತಿಜ್ಞಾ(man->ಗಂಟು != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ಸ್ಥಿರ nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		    ರಚನೆ mouse_action_state *mas)
{
	ರಚನೆ content *c = (ರಚನೆ content *)html;
	textarea_mouse_ಸ್ಥಿತಿ ta_ಸ್ಥಿತಿ;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ಆಯ್ಕೆ (mas->gadget.control->type) {
	ಸ್ಥಿತಿ GADGET_SELECT:
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormSelect");
		mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_MENU;
		ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ಆದರೆ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ಖಾಲಿ;
			}
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
			ಸಂದೇಶ_ದತ್ತಾಂಶ.select_menu.gadget = mas->gadget.control;
			content_ಪ್ರಸಾರ(c,
					  CONTENT_ಸಂದೇಶ_SELECTMENU,
					  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_CHECKBOX:
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormCheckbox");
		ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_ಒಳಹರಿವು_element_ಗಣ_checked(
				(dom_html_ಒಳಹರಿವು_element *)(mas->gadget.control->ಗಂಟು),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_RADIO:
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormRadio");
		ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_ಗಣ(mas->gadget.control);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
			ರಚನೆ image_ಒಳಹರಿವು_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*coords));
			ಆದರೆ (coords == ಖಾಲಿ) {
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ಆದರೆ (dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(
				mas->gadget.control->ಗಂಟು,
				corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_ದತ್ತಾಂಶ,
				coords,
				html_image_coords_dom_user_ದತ್ತಾಂಶ_handler,
				&oldcoords) != DOM_NO_ERR) {
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			}
			ಬಿಡುಗಡೆ(oldcoords);
		}
		fallthrough;

	ಸ್ಥಿತಿ GADGET_SUBMIT:
		ಆದರೆ (mas->gadget.control->form) {
			ಸ್ಥಿರ ಅಕ್ಷರ ಸ್ಥಿತಿ_ಬಫರ್[200];

			snprintf(ಸ್ಥಿತಿ_ಬಫರ್,
				 ಗಾತ್ರ ಸ್ಥಿತಿ_ಬಫರ್,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_ಬಫರ್;
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = get_ಸೂಚಕ_shape(mas->gadget.box,
								false);
			ಆದರೆ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->ಫಲಿತಾಂಶ.action = ACTION_SUBMIT;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormBadSubmit");
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_TEXTBOX:
	ಸ್ಥಿತಿ GADGET_PASSWORD:
	ಸ್ಥಿತಿ GADGET_TEXTAREA:
		ಆದರೆ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormTextarea");
		} ಇಲ್ಲದಿದ್ದರೆ {
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormTextbox");
		}

		ಆದರೆ (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			ಒಕ್ಕೂಟ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_ಗಣ_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_ಸ್ಥಿತಿ = textarea_mouse_action(mas->gadget.control->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		ಆದರೆ (ta_ಸ್ಥಿತಿ & TEXTAREA_MOUSE_EDITOR) {
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = get_ಸೂಚಕ_shape(mas->gadget.box, false);
		} ಇಲ್ಲದಿದ್ದರೆ {
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = scrollbar_mouse_ಸ್ಥಿತಿ_ಗಮ್ಯ_ಸಂದೇಶ(ta_ಸ್ಥಿತಿ >> 3);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_HIDDEN:
		/* not possible: no box generated */
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_RESET:
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormReset");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_FILE:
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormFile");
		ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
			ಸಂದೇಶ_ದತ್ತಾಂಶ.gadget_click.gadget = mas->gadget.control;
			content_ಪ್ರಸಾರ(c,
					  CONTENT_ಸಂದೇಶ_GADGETCLICK,
					  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FormButton");
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ಸ್ಥಿರ nserror
iframe_mouse_action(ರಚನೆ browser_ಕಿಟಕಿ *bw,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		    ರಚನೆ mouse_action_state *mas)
{
	ಪೂರ್ಣಾಂಕ pos_x, pos_y;
	ದಶಮಾಂಶ scale;

	scale = browser_ಕಿಟಕಿ_get_scale(bw);

	browser_ಕಿಟಕಿ_get_position(mas->iframe, false, &pos_x, &pos_y);

	ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_ಕಿಟಕಿ_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} ಇಲ್ಲದಿದ್ದರೆ {
		browser_ಕಿಟಕಿ_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->ಫಲಿತಾಂಶ.action = ACTION_NOSEND;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ಸ್ಥಿರ nserror
html_object_mouse_action(html_content *html,
			 ರಚನೆ browser_ಕಿಟಕಿ *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			 ರಚನೆ mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ಆದರೆ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		ಒಕ್ಕೂಟ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_ಗಣ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} ಇಲ್ಲದಿದ್ದರೆ {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->ಫಲಿತಾಂಶ.action = ACTION_NOSEND;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
	ಆದರೆ (scheme != ಖಾಲಿ) {
		ಆದರೆ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ಹಿಂತಿರುಗಿ is_js;
}


/**
 * process mouse activity on a link
 */
ಸ್ಥಿರ nserror
link_mouse_action(html_content *html,
		  ರಚನೆ browser_ಕಿಟಕಿ *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		  ರಚನೆ mouse_action_state *mas)
{
	nserror res;
	ಅಕ್ಷರ *url_s = ಖಾಲಿ;
	size_t url_l = 0;
	ಸ್ಥಿರ ಅಕ್ಷರ ಸ್ಥಿತಿ_ಬಫರ್[200];
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

	ಆದರೆ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ಆದರೆ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ಖಾಲಿ;
		}
	}

	ಆದರೆ (mas->title) {
		snprintf(ಸ್ಥಿತಿ_ಬಫರ್,
			 ಗಾತ್ರ ಸ್ಥಿತಿ_ಬಫರ್,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} ಇಲ್ಲದಿದ್ದರೆ {
		snprintf(ಸ್ಥಿತಿ_ಬಫರ್,
			 ಗಾತ್ರ ಸ್ಥಿತಿ_ಬಫರ್,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ಆದರೆ (url_s != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(url_s);
	}

	mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_ಬಫರ್;

	mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = get_ಸೂಚಕ_shape(mas->link.box,
						mas->link.is_imagemap);

	ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_ಕಿಟಕಿ_navigate(bw,
					mas->link.url,
					content_get_url((ರಚನೆ content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ಖಾಲಿ,
					ಖಾಲಿ,
					ಖಾಲಿ);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.savelink.url = mas->link.url;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.savelink.title = mas->title;
		content_ಪ್ರಸಾರ((ರಚನೆ content *)html,
				  CONTENT_ಸಂದೇಶ_SAVELINK,
				  &ಸಂದೇಶ_ದತ್ತಾಂಶ);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ಆದರೆ (is_javascript_navigate_url(mas->link.url)) {
			mas->ಫಲಿತಾಂಶ.action = ACTION_JS;
		} ಇಲ್ಲದಿದ್ದರೆ {
			mas->ಫಲಿತಾಂಶ.action = ACTION_NAVIGATE;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ಆದರೆ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		ಒಕ್ಕೂಟ html_focus_owner fo;
		fo.self = true;
		html_ಗಣ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ಖಾಲಿ);
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ಸ್ಥಿರ nserror
default_mouse_action(html_content *html,
		  ರಚನೆ browser_ಕಿಟಕಿ *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		  ರಚನೆ mouse_action_state *mas)
{
	ರಚನೆ content *c = (ರಚನೆ content *)html;

	/* frame resizing */
	ಆದರೆ (browser_ಕಿಟಕಿ_ಚೌಕಟ್ಟು_resize_ಆರಂಭ(bw, mouse, x, y, &mas->ಫಲಿತಾಂಶ.ಸೂಚಕ)) {
		ಆದರೆ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("FrameDrag");
		}
		ಹಿಂತಿರುಗಿ default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	ಒಕ್ಕೂಟ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ಆದರೆ (click && html->focus_type != HTML_FOCUS_SELF) {
		ಒಕ್ಕೂಟ html_focus_owner fo;
		fo.self = true;
		html_ಗಣ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ಖಾಲಿ);
	}
	ಆದರೆ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_ಗಣ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಆದರೆ (mas->ಪಠ್ಯ.box) {
		ಪೂರ್ಣಾಂಕ pixel_ಸ್ಥಾನಾಂತರ;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->ಪಠ್ಯ.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->ಪಠ್ಯ.box->ಪಠ್ಯ,
				       mas->ಪಠ್ಯ.box->ಉದ್ದಳತೆ,
				       x - mas->ಪಠ್ಯ.box_x,
				       &idx,
				       &pixel_ಸ್ಥಾನಾಂತರ);

		ಆದರೆ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->ಪಠ್ಯ.box->byte_ಸ್ಥಾನಾಂತರ + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			ಒಕ್ಕೂಟ html_drag_owner drag_owner;

			ಆದರೆ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_ಗಣ_drag_type(html,
						   drag_type,
						   drag_owner,
						   ಖಾಲಿ);
				mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = messages_get("Selecting");
			}

			ಆದರೆ (selection_ಸಕ್ರಿಯ(html->sel)) {
				sel_owner.none = false;
				html_ಗಣ_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_ಗಣ_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ಹಿಂತಿರುಗಿ default_mouse_action_focus(html, mouse);
		}

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ಆದರೆ (selection_ಸಕ್ರಿಯ(html->sel)) {
		sel_owner.none = false;
		html_ಗಣ_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_ಗಣ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ಆದರೆ (mas->title) {
		mas->ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = mas->title;
	}

	ಆದರೆ (mouse & BROWSER_MOUSE_DRAG_1) {
		ಆದರೆ (mouse & BROWSER_MOUSE_MOD_2) {
			ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.type = CONTENT_SAVE_COMPLETE;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content = ಖಾಲಿ;
			content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (mas->drag_candidate == ಖಾಲಿ) {
				browser_ಕಿಟಕಿ_ಪುಟ_drag_ಆರಂಭ(bw, x, y);
			} ಇಲ್ಲದಿದ್ದರೆ {
				html_box_drag_ಆರಂಭ(mas->drag_candidate, x, y);
			}
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_MOVE;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_DRAG_2) {
		ಆದರೆ (mouse & BROWSER_MOUSE_MOD_2) {
			ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.type = CONTENT_SAVE_ಮೂಲ;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content = ಖಾಲಿ;
			content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (mas->drag_candidate == ಖಾಲಿ) {
				browser_ಕಿಟಕಿ_ಪುಟ_drag_ಆರಂಭ(bw, x, y);
			} ಇಲ್ಲದಿದ್ದರೆ {
				html_box_drag_ಆರಂಭ(mas->drag_candidate, x, y);
			}
			mas->ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_MOVE;
		}
	}


	ಹಿಂತಿರುಗಿ default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ಸ್ಥಿರ nserror
mouse_action_drag_none(html_content *html,
		       ರಚನೆ browser_ಕಿಟಕಿ *bw,
		       browser_mouse_state mouse,
		       ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	nserror res;
	ರಚನೆ content *c = (ರಚನೆ content *)html;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	lwc_string *ಮಾರ್ಗ;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ಸ್ಥಿರ ರಚನೆ mouse_action_state mas;

	res = get_mouse_action_ಗಂಟು(html, x, y, &mas);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.ಫಲಿತಾಂಶ.action = ACTION_ಹಿಂಭಾಗ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.ಫಲಿತಾಂಶ.action = ACTION_FORWARD;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mas.scroll.bar) {
		mas.ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2 = scrollbar_mouse_ಸ್ಥಿತಿ_ಗಮ್ಯ_ಸಂದೇಶ(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.ಫಲಿತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_DEFAULT;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((mas.object != ಖಾಲಿ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ಆದರೆ (mouse & BROWSER_MOUSE_DRAG_2) {
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.type = CONTENT_SAVE_NATIVE;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content = mas.object;
			content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mouse & BROWSER_MOUSE_DRAG_1) {
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.type = CONTENT_SAVE_ORIG;
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content = mas.object;
			content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		}

		/* \todo should have a drag-saving object msg */

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mas.iframe != ಖಾಲಿ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mas.html_object.box != ಖಾಲಿ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (mas.link.url != ಖಾಲಿ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} ಇಲ್ಲದಿದ್ದರೆ {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* send status and pointer message */
	ಆದರೆ (mas.ಫಲಿತಾಂಶ.action != ACTION_NOSEND) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = mas.ಫಲಿತಾಂಶ.ಸ್ಥಿತಿ_2;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

		ಸಂದೇಶ_ದತ್ತಾಂಶ.ಸೂಚಕ = mas.ಫಲಿತಾಂಶ.ಸೂಚಕ;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸೂಚಕ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}

	/* fire dom click event */
	ಆದರೆ (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ಗಂಟು, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ಆಯ್ಕೆ (mas.ಫಲಿತಾಂಶ.action) {
	ಸ್ಥಿತಿ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_ಕಿಟಕಿ_ಕಂಡುಹಿಡಿ_ಗುರಿ(bw,
							     mas.gadget.ಗುರಿ,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ACTION_NAVIGATE:
		res = browser_ಕಿಟಕಿ_navigate(
				browser_ಕಿಟಕಿ_ಕಂಡುಹಿಡಿ_ಗುರಿ(bw,
							   mas.link.ಗುರಿ,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ಖಾಲಿ,
				ಖಾಲಿ,
				ಖಾಲಿ);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ACTION_JS:
		ಮಾರ್ಗ = nsurl_get_component(mas.link.url, NSURL_ಮಾರ್ಗ);
		ಆದರೆ (ಮಾರ್ಗ != ಖಾಲಿ) {
			html_exec(c,
				  lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ),
				  lwc_string_ಉದ್ದಳತೆ(ಮಾರ್ಗ));
			lwc_string_unref(ಮಾರ್ಗ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ACTION_ಹಿಂಭಾಗ:
		res = browser_ಕಿಟಕಿ_history_ಹಿಂಭಾಗ(bw, false);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ACTION_FORWARD:
		res = browser_ಕಿಟಕಿ_history_forward(bw, false);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ACTION_NOSEND:
	ಸ್ಥಿತಿ ACTION_NONE:
		res = NSERROR_OK;
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ರಚನೆ content *c,
			 ರಚನೆ browser_ಕಿಟಕಿ *bw,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ಹಿಂತಿರುಗಿ html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ರಚನೆ content *c,
		  ರಚನೆ browser_ಕಿಟಕಿ *bw,
		  browser_mouse_state mouse,
		  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ಆದರೆ (html->visible_select_menu != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ಆಯ್ಕೆ (html->drag_type) {
	ಸ್ಥಿತಿ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_TEXTAREA_SELECTION:
	ಸ್ಥಿತಿ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_CONTENT_SELECTION:
	ಸ್ಥಿತಿ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Unknown content related drag type */
		ಪ್ರತಿಜ್ಞಾ(0 && "Unknown content related drag type");
	}

	ಆದರೆ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ರಚನೆ content *c, uint32_t ಕೀಲಿ)
{
	html_content *html = (html_content *) c;
	ರಚನೆ selection *sel = html->sel;

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
	ಆದರೆ (html->layout != ಖಾಲಿ && html->layout->ಗಂಟು != ಖಾಲಿ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ಗಂಟು, true, true, ಕೀಲಿ);
	}

	ಆಯ್ಕೆ (html->focus_type) {
	ಸ್ಥಿತಿ HTML_FOCUS_CONTENT:
		ಹಿಂತಿರುಗಿ content_keypress(html->focus_owner.content->object, ಕೀಲಿ);

	ಸ್ಥಿತಿ HTML_FOCUS_TEXTAREA:
		ಆದರೆ (box_textarea_keypress(html, html->focus_owner.textarea, ಕೀಲಿ) == NSERROR_OK) {
			ಹಿಂತಿರುಗಿ true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹಿಂತಿರುಗಿ false;
		}

	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Deal with it below */
		ನಿಲ್ಲಿಸು;
	}

	ಆಯ್ಕೆ (ಕೀಲಿ) {
	ಸ್ಥಿತಿ NS_ಕೀಲಿ_COPY_SELECTION:
		selection_copy_ಗಮ್ಯ_clipboard(sel);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_SELECT_ALL:
		selection_select_all(sel);
		ಹಿಂತಿರುಗಿ true;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ಹಿಂತಿರುಗಿ selection_clear(sel, true);
	}

	ಹಿಂತಿರುಗಿ false;
}


/**
 * Callback for in-page scrollbars.
 */
ಶೂನ್ಯ html_ಅತಿಪ್ರವಾಹ_scroll_callback(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		ರಚನೆ scrollbar_ಸಂದೇಶ_ದತ್ತಾಂಶ *scrollbar_ದತ್ತಾಂಶ)
{
	ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = client_ದತ್ತಾಂಶ;
	html_content *html = (html_content *)ದತ್ತಾಂಶ->c;
	ರಚನೆ box *box = ದತ್ತಾಂಶ->box;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	html_drag_type drag_type;
	ಒಕ್ಕೂಟ html_drag_owner drag_owner;

	ಆಯ್ಕೆ(scrollbar_ದತ್ತಾಂಶ->ಸಂದೇಶ) {
	ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_MOVED:

		ಆದರೆ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ನಿಲ್ಲಿಸು;
		}

		html__redraw_a_box(html, box);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_SCROLL_ಆರಂಭ:
	{
		ರಚನೆ rect rect = {
			.x0 = scrollbar_ದತ್ತಾಂಶ->x0,
			.y0 = scrollbar_ದತ್ತಾಂಶ->y0,
			.x1 = scrollbar_ದತ್ತಾಂಶ->x1,
			.y1 = scrollbar_ದತ್ತಾಂಶ->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_ದತ್ತಾಂಶ->scrollbar;
		html_ಗಣ_drag_type(html, drag_type, drag_owner, &rect);
	}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_SCROLL_ಪೂರ್ಣಗೊಂಡ:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_ಗಣ_drag_type(html, drag_type, drag_owner, ಖಾಲಿ);

		ಸಂದೇಶ_ದತ್ತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_AUTO;
		content_ಪ್ರಸಾರ(ದತ್ತಾಂಶ->c, CONTENT_ಸಂದೇಶ_ಸೂಚಕ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	}
}


/* Documented in html_internal.h */
ಶೂನ್ಯ html_ಗಣ_drag_type(html_content *html, html_drag_type drag_type,
		ಒಕ್ಕೂಟ html_drag_owner drag_owner, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *rect)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

	ಪ್ರತಿಜ್ಞಾ(html != ಖಾಲಿ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ಆಯ್ಕೆ (drag_type) {
	ಸ್ಥಿತಿ HTML_DRAG_NONE:
		ಪ್ರತಿಜ್ಞಾ(drag_owner.no_owner == true);
		ಸಂದೇಶ_ದತ್ತಾಂಶ.drag.type = CONTENT_DRAG_NONE;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_SCROLLBAR:
	ಸ್ಥಿತಿ HTML_DRAG_TEXTAREA_SCROLLBAR:
	ಸ್ಥಿತಿ HTML_DRAG_CONTENT_SCROLL:
		ಸಂದೇಶ_ದತ್ತಾಂಶ.drag.type = CONTENT_DRAG_SCROLL;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_DRAG_SELECTION:
		ಪ್ರತಿಜ್ಞಾ(drag_owner.no_owner == true);
		fallthrough;
	ಸ್ಥಿತಿ HTML_DRAG_TEXTAREA_SELECTION:
	ಸ್ಥಿತಿ HTML_DRAG_CONTENT_SELECTION:
		ಸಂದೇಶ_ದತ್ತಾಂಶ.drag.type = CONTENT_DRAG_SELECTION;
		ನಿಲ್ಲಿಸು;
	}
	ಸಂದೇಶ_ದತ್ತಾಂಶ.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_ಪ್ರಸಾರ((ರಚನೆ content *)html, CONTENT_ಸಂದೇಶ_DRAG, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
}

/* Documented in html_internal.h */
ಶೂನ್ಯ html_ಗಣ_focus(html_content *html, html_focus_type focus_type,
		ಒಕ್ಕೂಟ html_focus_owner focus_owner, bool hide_caret,
		ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ x_off = 0;
	ಪೂರ್ಣಾಂಕ y_off = 0;
	ರಚನೆ rect cr;
	bool textarea_ಕಳೆದುಹೋದ_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ಪ್ರತಿಜ್ಞಾ(html != ಖಾಲಿ);

	ಆಯ್ಕೆ (focus_type) {
	ಸ್ಥಿತಿ HTML_FOCUS_SELF:
		ಪ್ರತಿಜ್ಞಾ(focus_owner.self == true);
		ಆದರೆ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ಹಿಂತಿರುಗಿ;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ನಿಲ್ಲಿಸು;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ಆದರೆ (textarea_ಕಳೆದುಹೋದ_focus) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.type = CONTENT_CARET_REMOVE;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.type = CONTENT_CARET_HIDE;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (clip != ಖಾಲಿ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.type = CONTENT_CARET_ಗಣ_POS;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.pos.x = x + x_off;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.pos.y = y + y_off;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.pos.ಎತ್ತರ = ಎತ್ತರ;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.caret.pos.clip = (clip == ಖಾಲಿ) ? ಖಾಲಿ : &cr;
	}

	/* Inform of the content's drag status change */
	content_ಪ್ರಸಾರ((ರಚನೆ content *)html, CONTENT_ಸಂದೇಶ_CARET, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
}

/* Documented in html_internal.h */
ಶೂನ್ಯ html_ಗಣ_selection(html_content *html, html_selection_type selection_type,
		ಒಕ್ಕೂಟ html_selection_owner selection_owner, bool read_only)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ರಚನೆ box *box;
	bool ಬದಲಾಯಿತು = false;
	bool same_type = html->selection_type == selection_type;

	ಪ್ರತಿಜ್ಞಾ(html != ಖಾಲಿ);

	ಆದರೆ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		ಬದಲಾಯಿತು = true;

	/* Clear any existing selection */
	ಆದರೆ (html->selection_type != HTML_SELECTION_NONE) {
		ಆಯ್ಕೆ (html->selection_type) {
		ಸ್ಥಿತಿ HTML_SELECTION_SELF:
			ಆದರೆ (same_type)
				ನಿಲ್ಲಿಸು;
			selection_clear(html->sel, true);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ HTML_SELECTION_TEXTAREA:
			ಆದರೆ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ನಿಲ್ಲಿಸು;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ HTML_SELECTION_CONTENT:
			ಆದರೆ (same_type && html->selection_owner.content ==
					selection_owner.content)
				ನಿಲ್ಲಿಸು;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ಆದರೆ (!ಬದಲಾಯಿತು)
		/* Don't need to report lack of change to owner */
		ಹಿಂತಿರುಗಿ;

	/* Prepare msg */
	ಆಯ್ಕೆ (selection_type) {
	ಸ್ಥಿತಿ HTML_SELECTION_NONE:
		ಪ್ರತಿಜ್ಞಾ(selection_owner.none == true);
		ಸಂದೇಶ_ದತ್ತಾಂಶ.selection.selection = false;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(selection_owner.none == false);
		fallthrough;
	ಸ್ಥಿತಿ HTML_SELECTION_TEXTAREA:
	ಸ್ಥಿತಿ HTML_SELECTION_CONTENT:
		ಸಂದೇಶ_ದತ್ತಾಂಶ.selection.selection = true;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
	ಸಂದೇಶ_ದತ್ತಾಂಶ.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_ಪ್ರಸಾರ((ರಚನೆ content *)html, CONTENT_ಸಂದೇಶ_SELECTION,
			&ಸಂದೇಶ_ದತ್ತಾಂಶ);
}
