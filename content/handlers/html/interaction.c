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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/keypress.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/browser_history.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/frames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/imagemap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ਸਥਿਰ browser_ਸੂਚਕ_shape get_ਸੂਚਕ_shape(ਬਣਤਰ box *box, bool imagemap)
{
	browser_ਸੂਚਕ_shape ਸੂਚਕ;
	css_computed_style *style;
	ਗਿਣਤੀ css_cursor_e cursor;
	lwc_string **cursor_uris;

	ਜੇ (box->type == BOX_FLOAT_ਖੱਬਾ || box->type == BOX_FLOAT_ਸੱਜਾ)
		style = box->children->style;
	ਨਹੀਂਤਾਂ
		style = box->style;

	ਜੇ (style == ਨਿਰਾ)
		ਵਾਪਸ BROWSER_ਸੂਚਕ_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ਚੋਣ (cursor) {
	ਹਾਲਤ CSS_CURSOR_AUTO:
		ਜੇ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਬਿੰਦੂ;
		} ਨਹੀਂਤਾਂ ਜੇ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			ਸੂਚਕ = BROWSER_ਸੂਚਕ_CARET;
		} ਨਹੀਂਤਾਂ {
			/* html content doesn't mind */
			ਸੂਚਕ = BROWSER_ਸੂਚਕ_AUTO;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_CROSSHAIR:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_CROSS;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਸੂਚਕ:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਬਿੰਦੂ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_MOVE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_MOVE;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_E_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਸੱਜਾ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_W_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਖੱਬਾ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_N_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਉੱਪਰ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_S_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਥੱਲੇ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_NE_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_RU;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_SW_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_LD;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_SE_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_RD;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_NW_RESIZE:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_LU;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਪਾਠ:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_CARET;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_WAIT:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_WAIT;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਤਰੱਕੀ:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_ਤਰੱਕੀ;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_HELP:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_HELP;
		ਰੋਕੋ;
	ਮੂਲ:
		ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਸੂਚਕ;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ਸਥਿਰ ਖਾਲੀ html_box_drag_ਸ਼ੁਰੂ(ਬਣਤਰ box *box, ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਪੂਰਨ box_x, box_y;
	ਪੂਰਨ scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ਜੇ (box->scroll_x != ਨਿਰਾ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ਸਿਖਰ_2] +
				box->ਉਚਾਈ + box->padding[BOTTOM] -
				SCROLLBAR_ਚੌੜਾਈ);
		scrollbar_ਸ਼ੁਰੂ_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} ਨਹੀਂਤਾਂ ਜੇ (box->scroll_y != ਨਿਰਾ) {
		scroll_mouse_x = x - (box_x + box->padding[ਖੱਬਾ_2] +
				box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] -
				SCROLLBAR_ਚੌੜਾਈ);
		scroll_mouse_y = y - box_y;

		scrollbar_ਸ਼ੁਰੂ_content_drag(box->scroll_y,
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
ਸਥਿਰ size_t html_selection_drag_ਸਿਰਾ(ਬਣਤਰ html_content *html,
		browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ dir)
{
	ਪੂਰਨ pixel_ਵਿੱਥ;
	ਬਣਤਰ box *box;
	ਪੂਰਨ dx, dy;
	size_t idx = 0;

	box = box_pick_ਪਾਠ_box(html, x, y, dir, &dx, &dy);
	ਜੇ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->ਪਾਠ, box->ਲੰਮਾਈ,
				       dx, &idx, &pixel_ਵਿੱਥ);

		idx += box->byte_ਵਿੱਥ;
	}

	ਵਾਪਸ idx;
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
ਸਥਿਰ ਖਾਲੀ
html_image_coords_dom_user_ਅੰਕੜੇ_handler(dom_ਗੰਢ_operation operation,
					 dom_string *ਕੁੰਜੀ,
					 ਖਾਲੀ *ਅੰਕੜੇ_2,
					 ਬਣਤਰ dom_ਗੰਢ *src,
					 ਬਣਤਰ dom_ਗੰਢ *dst)
{
	ਬਣਤਰ image_ਨਿਵੇਸ਼_coords *oldcoords, *coords = ਅੰਕੜੇ_2, *newcoords;

	ਜੇ (!dom_string_isequal(corestring_dom_ns_ਕੁੰਜੀ_image_coords_ਗੰਢ_ਅੰਕੜੇ,
				ਕੁੰਜੀ) || coords == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ਚੋਣ (operation) {
	ਹਾਲਤ DOM_ਗੰਢ_CLONED:
		newcoords = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*newcoords));
		ਜੇ (newcoords != ਨਿਰਾ) {
			*newcoords = *coords;
			ਜੇ (dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(dst,
				 corestring_dom_ns_ਕੁੰਜੀ_image_coords_ਗੰਢ_ਅੰਕੜੇ,
				 newcoords,
				 html_image_coords_dom_user_ਅੰਕੜੇ_handler,
				 &oldcoords) == DOM_NO_ERR) {
				ਮੁਕਤ(oldcoords);
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_DELETED:
		ਮੁਕਤ(coords);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_RENAMED:
	ਹਾਲਤ DOM_ਗੰਢ_IMPORTED:
	ਹਾਲਤ DOM_ਗੰਢ_ADOPTED:
		ਰੋਕੋ;

	ਮੂਲ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
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
ਸਥਿਰ ਖਾਲੀ
html_ਉਛਾਲ_scroll_drag_ਸਿਰਾ(ਬਣਤਰ scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਪੂਰਨ scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ *ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(scrollbar);
	ਬਣਤਰ box *box;

	box = ਅੰਕੜੇ->box;
	box_coords(box, &box_x, &box_y);

	ਜੇ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[ਸਿਖਰ_2] +
				box->ਉਚਾਈ + box->padding[BOTTOM] -
				SCROLLBAR_ਚੌੜਾਈ);
		scrollbar_mouse_drag_ਸਿਰਾ(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} ਨਹੀਂਤਾਂ {
		scroll_mouse_x = x - (box_x + box->padding[ਖੱਬਾ_2] +
				box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] -
				SCROLLBAR_ਚੌੜਾਈ);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_ਸਿਰਾ(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ਸਥਿਰ nserror
mouse_action_select_menu(html_content *html,
			 ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			 browser_mouse_state mouse,
			 ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ box_x = 0;
	ਪੂਰਨ box_y = 0;
	ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
	ਬਣਤਰ hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ਪ੍ਰਤਿਜ੍ਞਾ(html->visible_select_menu != ਨਿਰਾ);

	bw_drag_type = browser_ਖਿੜਕੀ_get_drag_type(bw);
	ਜੇ (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_ਸਿਰਾ(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	box_y += box->ਉਚਾਈ + box->border[BOTTOM].ਚੌੜਾਈ +
		box->padding[BOTTOM] + box->padding[ਸਿਖਰ_2];

	ਸਥਿਤੀ = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ਜੇ (ਸਥਿਤੀ != ਨਿਰਾ) {
		/* set status if menu still open */
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਸਥਿਤੀ;
		content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html,
				  CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ,
				  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਵਾਪਸ NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &ਚੌੜਾਈ, &ਉਚਾਈ);

	html->visible_select_menu = ਨਿਰਾ;

	bw_content = browser_ਖਿੜਕੀ_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       ਚੌੜਾਈ,
			       ਉਚਾਈ);
	ਵਾਪਸ NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ਸਥਿਰ nserror
mouse_action_drag_selection(html_content *html,
			    ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			    browser_mouse_state mouse,
			    ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ dir = -1;
	ਪੂਰਨ dx, dy;
	size_t idx;
	ਸੰਘ html_drag_owner drag_owner;
	ਪੂਰਨ pixel_ਵਿੱਥ;
	plot_font_style_t fstyle;

	ਜੇ (!mouse) {
		/* End of selection drag */
		ਜੇ (selection_dragging_ਸ਼ੁਰੂ(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_ਸਿਰਾ(html, mouse, x, y, dir);

		ਜੇ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_ਸੈੱਟ_drag_type(html, HTML_DRAG_NONE, drag_owner, ਨਿਰਾ);

		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (selection_dragging_ਸ਼ੁਰੂ(html->sel)) {
		dir = 1;
	}

	box = box_pick_ਪਾਠ_box(html, x, y, dir, &dx, &dy);
	ਜੇ (box != ਨਿਰਾ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->ਪਾਠ,
				       box->ਲੰਮਾਈ,
				       dx,
				       &idx,
				       &pixel_ਵਿੱਥ);

		selection_track(html->sel, mouse, box->byte_ਵਿੱਥ + idx);
	}
	ਵਾਪਸ NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ਸਥਿਰ nserror
mouse_action_drag_scrollbar(html_content *html,
			    ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			    browser_mouse_state mouse,
			    ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ scrollbar *scr;
	ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਬਣਤਰ box *box;
	ਪੂਰਨ box_x = 0;
	ਪੂਰਨ box_y = 0;
	ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ;
	ਪੂਰਨ scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_ਸਥਿਤੀ scrollbar_ਸਥਿਤੀ;

	scr = html->drag_owner.scrollbar;

	ਜੇ (!mouse) {
		/* drag end: scrollbar */
		html_ਉਛਾਲ_scroll_drag_ਸਿਰਾ(scr, mouse, x, y);
	}

	ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(scr);

	box = ਅੰਕੜੇ->box;

	box_coords(box, &box_x, &box_y);

	ਜੇ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ਸਿਖਰ_2] +
				      box->ਉਚਾਈ + box->padding[BOTTOM] -
				      SCROLLBAR_ਚੌੜਾਈ);
		scrollbar_ਸਥਿਤੀ = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} ਨਹੀਂਤਾਂ {
		scroll_mouse_x = x - (box_x + box->padding[ਖੱਬਾ_2] +
				      box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] -
				      SCROLLBAR_ਚੌੜਾਈ);
		scroll_mouse_y = y - box_y;

		scrollbar_ਸਥਿਤੀ = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	ਸਥਿਤੀ = scrollbar_mouse_ਸਥਿਤੀ_ਮੰਜ਼ਿਲ_ਸੁਨੇਹਾ(scrollbar_ਸਥਿਤੀ);

	ਜੇ (ਸਥਿਤੀ != ਨਿਰਾ) {
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਸਥਿਤੀ;
		content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html,
				  CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ,
				  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ਸਥਿਰ nserror
mouse_action_drag_textarea(html_content *html,
			    ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			    browser_mouse_state mouse,
			    ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ box_x = 0;
	ਪੂਰਨ box_y = 0;

	box = html->drag_owner.textarea;

	ਪ੍ਰਤਿਜ੍ਞਾ(box->gadget != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ਵਾਪਸ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ਸਥਿਰ nserror
mouse_action_drag_content(html_content *html,
			  ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			  browser_mouse_state mouse,
			  ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ box_x = 0;
	ਪੂਰਨ box_y = 0;

	box = html->drag_owner.content;
	ਪ੍ਰਤਿਜ੍ਞਾ(box->object != ਨਿਰਾ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ਵਾਪਸ NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ਬਣਤਰ mouse_action_state {
	ਬਣਤਰ {
		ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ; /**< status text */
		browser_ਸੂਚਕ_shape ਸੂਚਕ; /**< pointer shape */
		ਗਿਣਤੀ {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_ਪਿੱਠ, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} ਨਤੀਜਾ;

	/** dom node */
	ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ;

	/** html object */
	ਬਣਤਰ {
		ਬਣਤਰ box *box;
		ਪੂਰਨ pos_x;
		ਪੂਰਨ pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ਬਣਤਰ browser_ਖਿੜਕੀ *iframe;

	/** link either from href or imagemap */
	ਬਣਤਰ {
		ਬਣਤਰ box *box;
		nsurl *url;
		ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;
		bool is_imagemap;
	} link;

	/** gadget */
	ਬਣਤਰ {
		ਬਣਤਰ form_control *control;
		ਬਣਤਰ box *box;
		ਪੂਰਨ box_x;
		ਪੂਰਨ box_y;
		ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;
	} gadget;

	/** title */
	ਅਚਲ ਅੱਖਰ *title;

	/** candidate box for drag operation */
	ਬਣਤਰ box *drag_candidate;

	/** scrollbar */
	ਬਣਤਰ {
		ਬਣਤਰ scrollbar *bar;
		ਪੂਰਨ mouse_x;
		ਪੂਰਨ mouse_y;
	} scroll;

	/** text in box */
	ਬਣਤਰ {
		ਬਣਤਰ box *box;
		ਪੂਰਨ box_x;
	} ਪਾਠ;
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
ਸਥਿਰ nserror
get_mouse_action_ਗੰਢ(html_content *html,
		      ਪੂਰਨ x, ਪੂਰਨ y,
		      ਬਣਤਰ mouse_action_state *man)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ box_x = 0;
	ਪੂਰਨ box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, ਆਕਾਰ(ਬਣਤਰ mouse_action_state));
	man->ਗੰਢ = html->layout->ਗੰਢ; /* Default dom node to the <HTML> */
	man->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ਖੱਬਾ_2];
	box_y = box->margin[ਸਿਖਰ_2];

	ਕਰੋ {
		/* skip hidden boxes */
		ਜੇ ((box->style != ਨਿਰਾ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			ਜਾਓ ਅਗਲਾ_box;
		}

		ਜੇ (box->ਗੰਢ != ਨਿਰਾ) {
			man->ਗੰਢ = box->ਗੰਢ;
		}

		ਜੇ (box->object) {
			ਜੇ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} ਨਹੀਂਤਾਂ {
				man->object = box->object;
			}
		}

		ਜੇ (box->iframe) {
			man->iframe = box->iframe;
		}

		ਜੇ (box->href) {
			man->link.url = box->href;
			man->link.ਨਿਸ਼ਾਨਾ = box->ਨਿਸ਼ਾਨਾ;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ਜੇ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.ਨਿਸ਼ਾਨਾ);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ਜੇ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ਜੇ (box->gadget->form) {
				man->gadget.ਨਿਸ਼ਾਨਾ = box->gadget->form->ਨਿਸ਼ਾਨਾ;
			}
		}

		ਜੇ (box->title) {
			man->title = box->title;
		}

		man->ਨਤੀਜਾ.ਸੂਚਕ = get_ਸੂਚਕ_shape(box, false);

		ਜੇ ((box->scroll_x != ਨਿਰਾ) ||
		    (box->scroll_y != ਨਿਰਾ)) {
			ਪੂਰਨ padding_ਖੱਬਾ;
			ਪੂਰਨ padding_ਸੱਜਾ;
			ਪੂਰਨ padding_ਸਿਖਰ;
			ਪੂਰਨ padding_bottom;

			ਜੇ (man->drag_candidate == ਨਿਰਾ) {
				man->drag_candidate = box;
			}

			padding_ਖੱਬਾ = box_x +
					scrollbar_get_ਵਿੱਥ(box->scroll_x);
			padding_ਸੱਜਾ = padding_ਖੱਬਾ + box->padding[ਖੱਬਾ_2] +
					box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
			padding_ਸਿਖਰ = box_y +
					scrollbar_get_ਵਿੱਥ(box->scroll_y);
			padding_bottom = padding_ਸਿਖਰ + box->padding[ਸਿਖਰ_2] +
					box->ਉਚਾਈ + box->padding[BOTTOM];

			ਜੇ ((x > padding_ਖੱਬਾ) &&
			    (x < padding_ਸੱਜਾ) &&
			    (y > padding_ਸਿਖਰ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ਜੇ ((box->scroll_y != ਨਿਰਾ) &&
				    (x > (padding_ਸੱਜਾ - SCROLLBAR_ਚੌੜਾਈ))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ਸੱਜਾ - SCROLLBAR_ਚੌੜਾਈ);
					man->scroll.mouse_y = y - padding_ਸਿਖਰ;
					ਰੋਕੋ;

				} ਨਹੀਂਤਾਂ ਜੇ ((box->scroll_x != ਨਿਰਾ) &&
					   (y > (padding_bottom -
							SCROLLBAR_ਚੌੜਾਈ))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ਖੱਬਾ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_ਚੌੜਾਈ);
					ਰੋਕੋ;
				}
			}
		}

		ਜੇ (box->ਪਾਠ && !box->object) {
			man->ਪਾਠ.box = box;
			man->ਪਾਠ.box_x = box_x;
		}

	ਅਗਲਾ_box:
		/* iterate to next box */
		box = box_at_ਬਿੰਦੂ(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ਜਦੋਂ (box != ਨਿਰਾ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ਪ੍ਰਤਿਜ੍ਞਾ(man->ਗੰਢ != ਨਿਰਾ);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ਸਥਿਰ nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    ਪੂਰਨ x, ਪੂਰਨ y,
		    ਬਣਤਰ mouse_action_state *mas)
{
	ਬਣਤਰ content *c = (ਬਣਤਰ content *)html;
	textarea_mouse_ਸਥਿਤੀ ta_ਸਥਿਤੀ;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ਚੋਣ (mas->gadget.control->type) {
	ਹਾਲਤ GADGET_SELECT:
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormSelect");
		mas->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_MENU;
		ਜੇ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ਜੇ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ਨਿਰਾ;
			}
			mas->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;
		} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.select_menu.gadget = mas->gadget.control;
			content_ਪ੍ਰਸਾਰਣ(c,
					  CONTENT_ਸੁਨੇਹਾ_SELECTMENU,
					  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_CHECKBOX:
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormCheckbox");
		ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_ਨਿਵੇਸ਼_element_ਸੈੱਟ_checked(
				(dom_html_ਨਿਵੇਸ਼_element *)(mas->gadget.control->ਗੰਢ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_RADIO:
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormRadio");
		ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_ਸੈੱਟ(mas->gadget.control);
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
			ਬਣਤਰ image_ਨਿਵੇਸ਼_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*coords));
			ਜੇ (coords == ਨਿਰਾ) {
				ਵਾਪਸ NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ਜੇ (dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(
				mas->gadget.control->ਗੰਢ,
				corestring_dom_ns_ਕੁੰਜੀ_image_coords_ਗੰਢ_ਅੰਕੜੇ,
				coords,
				html_image_coords_dom_user_ਅੰਕੜੇ_handler,
				&oldcoords) != DOM_NO_ERR) {
				ਵਾਪਸ NSERROR_OK;
			}
			ਮੁਕਤ(oldcoords);
		}
		fallthrough;

	ਹਾਲਤ GADGET_SUBMIT:
		ਜੇ (mas->gadget.control->form) {
			ਸਥਿਰ ਅੱਖਰ ਸਥਿਤੀ_ਬਫਰ[200];

			snprintf(ਸਥਿਤੀ_ਬਫਰ,
				 ਆਕਾਰ ਸਥਿਤੀ_ਬਫਰ,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = ਸਥਿਤੀ_ਬਫਰ;
			mas->ਨਤੀਜਾ.ਸੂਚਕ = get_ਸੂਚਕ_shape(mas->gadget.box,
								false);
			ਜੇ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->ਨਤੀਜਾ.action = ACTION_SUBMIT;
			}
		} ਨਹੀਂਤਾਂ {
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormBadSubmit");
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_TEXTBOX:
	ਹਾਲਤ GADGET_PASSWORD:
	ਹਾਲਤ GADGET_TEXTAREA:
		ਜੇ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormTextarea");
		} ਨਹੀਂਤਾਂ {
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormTextbox");
		}

		ਜੇ (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			ਸੰਘ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_ਸੈੱਟ_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_ਸਥਿਤੀ = textarea_mouse_action(mas->gadget.control->ਅੰਕੜੇ.ਪਾਠ.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		ਜੇ (ta_ਸਥਿਤੀ & TEXTAREA_MOUSE_EDITOR) {
			mas->ਨਤੀਜਾ.ਸੂਚਕ = get_ਸੂਚਕ_shape(mas->gadget.box, false);
		} ਨਹੀਂਤਾਂ {
			mas->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = scrollbar_mouse_ਸਥਿਤੀ_ਮੰਜ਼ਿਲ_ਸੁਨੇਹਾ(ta_ਸਥਿਤੀ >> 3);
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_HIDDEN:
		/* not possible: no box generated */
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_RESET:
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormReset");
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_FILE:
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormFile");
		ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.gadget_click.gadget = mas->gadget.control;
			content_ਪ੍ਰਸਾਰਣ(c,
					  CONTENT_ਸੁਨੇਹਾ_GADGETCLICK,
					  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FormButton");
		ਰੋਕੋ;
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ਸਥਿਰ nserror
iframe_mouse_action(ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		    browser_mouse_state mouse,
		    ਪੂਰਨ x, ਪੂਰਨ y,
		    ਬਣਤਰ mouse_action_state *mas)
{
	ਪੂਰਨ pos_x, pos_y;
	ਦਸ਼ਮਲਵ scale;

	scale = browser_ਖਿੜਕੀ_get_scale(bw);

	browser_ਖਿੜਕੀ_get_position(mas->iframe, false, &pos_x, &pos_y);

	ਜੇ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_ਖਿੜਕੀ_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} ਨਹੀਂਤਾਂ {
		browser_ਖਿੜਕੀ_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->ਨਤੀਜਾ.action = ACTION_NOSEND;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ਸਥਿਰ nserror
html_object_mouse_action(html_content *html,
			 ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			 browser_mouse_state mouse,
			 ਪੂਰਨ x, ਪੂਰਨ y,
			 ਬਣਤਰ mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ਜੇ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		ਸੰਘ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_ਸੈੱਟ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ਜੇ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} ਨਹੀਂਤਾਂ {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->ਨਤੀਜਾ.action = ACTION_NOSEND;
	ਵਾਪਸ NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ਸਥਿਰ bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (scheme != ਨਿਰਾ) {
		ਜੇ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ਵਾਪਸ is_js;
}


/**
 * process mouse activity on a link
 */
ਸਥਿਰ nserror
link_mouse_action(html_content *html,
		  ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		  browser_mouse_state mouse,
		  ਪੂਰਨ x, ਪੂਰਨ y,
		  ਬਣਤਰ mouse_action_state *mas)
{
	nserror res;
	ਅੱਖਰ *url_s = ਨਿਰਾ;
	size_t url_l = 0;
	ਸਥਿਰ ਅੱਖਰ ਸਥਿਤੀ_ਬਫਰ[200];
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

	ਜੇ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ਜੇ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ਨਿਰਾ;
		}
	}

	ਜੇ (mas->title) {
		snprintf(ਸਥਿਤੀ_ਬਫਰ,
			 ਆਕਾਰ ਸਥਿਤੀ_ਬਫਰ,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} ਨਹੀਂਤਾਂ {
		snprintf(ਸਥਿਤੀ_ਬਫਰ,
			 ਆਕਾਰ ਸਥਿਤੀ_ਬਫਰ,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ਜੇ (url_s != ਨਿਰਾ) {
		ਮੁਕਤ(url_s);
	}

	mas->ਨਤੀਜਾ.ਸਥਿਤੀ = ਸਥਿਤੀ_ਬਫਰ;

	mas->ਨਤੀਜਾ.ਸੂਚਕ = get_ਸੂਚਕ_shape(mas->link.box,
						mas->link.is_imagemap);

	ਜੇ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_ਖਿੜਕੀ_navigate(bw,
					mas->link.url,
					content_get_url((ਬਣਤਰ content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ਨਿਰਾ,
					ਨਿਰਾ,
					ਨਿਰਾ);

	} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.savelink.url = mas->link.url;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.savelink.title = mas->title;
		content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html,
				  CONTENT_ਸੁਨੇਹਾ_SAVELINK,
				  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

	} ਨਹੀਂਤਾਂ ਜੇ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ਜੇ (is_javascript_navigate_url(mas->link.url)) {
			mas->ਨਤੀਜਾ.action = ACTION_JS;
		} ਨਹੀਂਤਾਂ {
			mas->ਨਤੀਜਾ.action = ACTION_NAVIGATE;
		}
	}

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ਜੇ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		ਸੰਘ html_focus_owner fo;
		fo.self = true;
		html_ਸੈੱਟ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ਨਿਰਾ);
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ਸਥਿਰ nserror
default_mouse_action(html_content *html,
		  ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		  browser_mouse_state mouse,
		  ਪੂਰਨ x, ਪੂਰਨ y,
		  ਬਣਤਰ mouse_action_state *mas)
{
	ਬਣਤਰ content *c = (ਬਣਤਰ content *)html;

	/* frame resizing */
	ਜੇ (browser_ਖਿੜਕੀ_ਚੌਖਟਾ_resize_ਸ਼ੁਰੂ(bw, mouse, x, y, &mas->ਨਤੀਜਾ.ਸੂਚਕ)) {
		ਜੇ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("FrameDrag");
		}
		ਵਾਪਸ default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	ਸੰਘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ਜੇ (click && html->focus_type != HTML_FOCUS_SELF) {
		ਸੰਘ html_focus_owner fo;
		fo.self = true;
		html_ਸੈੱਟ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ਨਿਰਾ);
	}
	ਜੇ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_ਸੈੱਟ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ਜੇ (mas->ਪਾਠ.box) {
		ਪੂਰਨ pixel_ਵਿੱਥ;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->ਪਾਠ.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->ਪਾਠ.box->ਪਾਠ,
				       mas->ਪਾਠ.box->ਲੰਮਾਈ,
				       x - mas->ਪਾਠ.box_x,
				       &idx,
				       &pixel_ਵਿੱਥ);

		ਜੇ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->ਪਾਠ.box->byte_ਵਿੱਥ + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			ਸੰਘ html_drag_owner drag_owner;

			ਜੇ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_ਸੈੱਟ_drag_type(html,
						   drag_type,
						   drag_owner,
						   ਨਿਰਾ);
				mas->ਨਤੀਜਾ.ਸਥਿਤੀ = messages_get("Selecting");
			}

			ਜੇ (selection_ਸਰਗਰਮ(html->sel)) {
				sel_owner.none = false;
				html_ਸੈੱਟ_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} ਨਹੀਂਤਾਂ ਜੇ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_ਸੈੱਟ_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ਵਾਪਸ default_mouse_action_focus(html, mouse);
		}

	} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ਜੇ (selection_ਸਰਗਰਮ(html->sel)) {
		sel_owner.none = false;
		html_ਸੈੱਟ_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} ਨਹੀਂਤਾਂ ਜੇ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_ਸੈੱਟ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ਜੇ (mas->title) {
		mas->ਨਤੀਜਾ.ਸਥਿਤੀ = mas->title;
	}

	ਜੇ (mouse & BROWSER_MOUSE_DRAG_1) {
		ਜੇ (mouse & BROWSER_MOUSE_MOD_2) {
			ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.type = CONTENT_SAVE_COMPLETE;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content = ਨਿਰਾ;
			content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_DRAGSAVE, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (mas->drag_candidate == ਨਿਰਾ) {
				browser_ਖਿੜਕੀ_ਪੰਨਾ_drag_ਸ਼ੁਰੂ(bw, x, y);
			} ਨਹੀਂਤਾਂ {
				html_box_drag_ਸ਼ੁਰੂ(mas->drag_candidate, x, y);
			}
			mas->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_MOVE;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_DRAG_2) {
		ਜੇ (mouse & BROWSER_MOUSE_MOD_2) {
			ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.type = CONTENT_SAVE_ਸਰੋਤ;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content = ਨਿਰਾ;
			content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_DRAGSAVE, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (mas->drag_candidate == ਨਿਰਾ) {
				browser_ਖਿੜਕੀ_ਪੰਨਾ_drag_ਸ਼ੁਰੂ(bw, x, y);
			} ਨਹੀਂਤਾਂ {
				html_box_drag_ਸ਼ੁਰੂ(mas->drag_candidate, x, y);
			}
			mas->ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_MOVE;
		}
	}


	ਵਾਪਸ default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ਸਥਿਰ nserror
mouse_action_drag_none(html_content *html,
		       ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		       browser_mouse_state mouse,
		       ਪੂਰਨ x, ਪੂਰਨ y)
{
	nserror res;
	ਬਣਤਰ content *c = (ਬਣਤਰ content *)html;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	lwc_string *ਰਸਤਾ;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ਸਥਿਰ ਬਣਤਰ mouse_action_state mas;

	res = get_mouse_action_ਗੰਢ(html, x, y, &mas);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਜੇ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.ਨਤੀਜਾ.action = ACTION_ਪਿੱਠ;
	} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.ਨਤੀਜਾ.action = ACTION_FORWARD;
	} ਨਹੀਂਤਾਂ ਜੇ (mas.scroll.bar) {
		mas.ਨਤੀਜਾ.ਸਥਿਤੀ = scrollbar_mouse_ਸਥਿਤੀ_ਮੰਜ਼ਿਲ_ਸੁਨੇਹਾ(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.ਨਤੀਜਾ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_DEFAULT;

	} ਨਹੀਂਤਾਂ ਜੇ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} ਨਹੀਂਤਾਂ ਜੇ ((mas.object != ਨਿਰਾ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ਜੇ (mouse & BROWSER_MOUSE_DRAG_2) {
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.type = CONTENT_SAVE_NATIVE;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content = mas.object;
			content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_DRAGSAVE, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

		} ਨਹੀਂਤਾਂ ਜੇ (mouse & BROWSER_MOUSE_DRAG_1) {
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.type = CONTENT_SAVE_ORIG;
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content = mas.object;
			content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_DRAGSAVE, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		}

		/* \todo should have a drag-saving object msg */

	} ਨਹੀਂਤਾਂ ਜੇ (mas.iframe != ਨਿਰਾ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} ਨਹੀਂਤਾਂ ਜੇ (mas.html_object.box != ਨਿਰਾ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} ਨਹੀਂਤਾਂ ਜੇ (mas.link.url != ਨਿਰਾ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} ਨਹੀਂਤਾਂ {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	/* send status and pointer message */
	ਜੇ (mas.ਨਤੀਜਾ.action != ACTION_NOSEND) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = mas.ਨਤੀਜਾ.ਸਥਿਤੀ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.ਸੂਚਕ = mas.ਨਤੀਜਾ.ਸੂਚਕ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}

	/* fire dom click event */
	ਜੇ (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ਗੰਢ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ਚੋਣ (mas.ਨਤੀਜਾ.action) {
	ਹਾਲਤ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_ਖਿੜਕੀ_ਲੱਭੋ_ਨਿਸ਼ਾਨਾ(bw,
							     mas.gadget.ਨਿਸ਼ਾਨਾ,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ਰੋਕੋ;

	ਹਾਲਤ ACTION_NAVIGATE:
		res = browser_ਖਿੜਕੀ_navigate(
				browser_ਖਿੜਕੀ_ਲੱਭੋ_ਨਿਸ਼ਾਨਾ(bw,
							   mas.link.ਨਿਸ਼ਾਨਾ,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ਨਿਰਾ,
				ਨਿਰਾ,
				ਨਿਰਾ);
		ਰੋਕੋ;

	ਹਾਲਤ ACTION_JS:
		ਰਸਤਾ = nsurl_get_component(mas.link.url, NSURL_ਰਸਤਾ);
		ਜੇ (ਰਸਤਾ != ਨਿਰਾ) {
			html_exec(c,
				  lwc_string_ਅੰਕੜੇ(ਰਸਤਾ),
				  lwc_string_ਲੰਮਾਈ(ਰਸਤਾ));
			lwc_string_unref(ਰਸਤਾ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ ACTION_ਪਿੱਠ:
		res = browser_ਖਿੜਕੀ_history_ਪਿੱਠ(bw, false);
		ਰੋਕੋ;

	ਹਾਲਤ ACTION_FORWARD:
		res = browser_ਖਿੜਕੀ_history_forward(bw, false);
		ਰੋਕੋ;

	ਹਾਲਤ ACTION_NOSEND:
	ਹਾਲਤ ACTION_NONE:
		res = NSERROR_OK;
		ਰੋਕੋ;
	}

	ਵਾਪਸ res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ਬਣਤਰ content *c,
			 ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			 browser_mouse_state mouse,
			 ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਵਾਪਸ html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ਬਣਤਰ content *c,
		  ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		  browser_mouse_state mouse,
		  ਪੂਰਨ x, ਪੂਰਨ y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ਜੇ (html->visible_select_menu != ਨਿਰਾ) {
		ਵਾਪਸ mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ਚੋਣ (html->drag_type) {
	ਹਾਲਤ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_TEXTAREA_SELECTION:
	ਹਾਲਤ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_CONTENT_SELECTION:
	ਹਾਲਤ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ਰੋਕੋ;

	ਮੂਲ:
		/* Unknown content related drag type */
		ਪ੍ਰਤਿਜ੍ਞਾ(0 && "Unknown content related drag type");
	}

	ਜੇ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ਵਾਪਸ res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ਬਣਤਰ content *c, uint32_t ਕੁੰਜੀ)
{
	html_content *html = (html_content *) c;
	ਬਣਤਰ selection *sel = html->sel;

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
	ਜੇ (html->layout != ਨਿਰਾ && html->layout->ਗੰਢ != ਨਿਰਾ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ਗੰਢ, true, true, ਕੁੰਜੀ);
	}

	ਚੋਣ (html->focus_type) {
	ਹਾਲਤ HTML_FOCUS_CONTENT:
		ਵਾਪਸ content_keypress(html->focus_owner.content->object, ਕੁੰਜੀ);

	ਹਾਲਤ HTML_FOCUS_TEXTAREA:
		ਜੇ (box_textarea_keypress(html, html->focus_owner.textarea, ਕੁੰਜੀ) == NSERROR_OK) {
			ਵਾਪਸ true;
		} ਨਹੀਂਤਾਂ {
			ਵਾਪਸ false;
		}

	ਮੂਲ:
		/* Deal with it below */
		ਰੋਕੋ;
	}

	ਚੋਣ (ਕੁੰਜੀ) {
	ਹਾਲਤ NS_ਕੁੰਜੀ_COPY_SELECTION:
		selection_copy_ਮੰਜ਼ਿਲ_clipboard(sel);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_CLEAR_SELECTION:
		selection_clear(sel, true);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_SELECT_ALL:
		selection_select_all(sel);
		ਵਾਪਸ true;

	ਹਾਲਤ NS_ਕੁੰਜੀ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ਵਾਪਸ selection_clear(sel, true);
	}

	ਵਾਪਸ false;
}


/**
 * Callback for in-page scrollbars.
 */
ਖਾਲੀ html_ਉਛਾਲ_scroll_callback(ਖਾਲੀ *client_ਅੰਕੜੇ,
		ਬਣਤਰ scrollbar_ਸੁਨੇਹਾ_ਅੰਕੜੇ *scrollbar_ਅੰਕੜੇ)
{
	ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ *ਅੰਕੜੇ = client_ਅੰਕੜੇ;
	html_content *html = (html_content *)ਅੰਕੜੇ->c;
	ਬਣਤਰ box *box = ਅੰਕੜੇ->box;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	html_drag_type drag_type;
	ਸੰਘ html_drag_owner drag_owner;

	ਚੋਣ(scrollbar_ਅੰਕੜੇ->ਸੁਨੇਹਾ) {
	ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_MOVED:

		ਜੇ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ਰੋਕੋ;
		}

		html__redraw_a_box(html, box);
		ਰੋਕੋ;
	ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_SCROLL_ਸ਼ੁਰੂ:
	{
		ਬਣਤਰ rect rect = {
			.x0 = scrollbar_ਅੰਕੜੇ->x0,
			.y0 = scrollbar_ਅੰਕੜੇ->y0,
			.x1 = scrollbar_ਅੰਕੜੇ->x1,
			.y1 = scrollbar_ਅੰਕੜੇ->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_ਅੰਕੜੇ->scrollbar;
		html_ਸੈੱਟ_drag_type(html, drag_type, drag_owner, &rect);
	}
		ਰੋਕੋ;
	ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_SCROLL_ਮੁਕੰਮਲ:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_ਸੈੱਟ_drag_type(html, drag_type, drag_owner, ਨਿਰਾ);

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_AUTO;
		content_ਪ੍ਰਸਾਰਣ(ਅੰਕੜੇ->c, CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਰੋਕੋ;
	}
}


/* Documented in html_internal.h */
ਖਾਲੀ html_ਸੈੱਟ_drag_type(html_content *html, html_drag_type drag_type,
		ਸੰਘ html_drag_owner drag_owner, ਅਚਲ ਬਣਤਰ rect *rect)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

	ਪ੍ਰਤਿਜ੍ਞਾ(html != ਨਿਰਾ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ਚੋਣ (drag_type) {
	ਹਾਲਤ HTML_DRAG_NONE:
		ਪ੍ਰਤਿਜ੍ਞਾ(drag_owner.no_owner == true);
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.drag.type = CONTENT_DRAG_NONE;
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_SCROLLBAR:
	ਹਾਲਤ HTML_DRAG_TEXTAREA_SCROLLBAR:
	ਹਾਲਤ HTML_DRAG_CONTENT_SCROLL:
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.drag.type = CONTENT_DRAG_SCROLL;
		ਰੋਕੋ;

	ਹਾਲਤ HTML_DRAG_SELECTION:
		ਪ੍ਰਤਿਜ੍ਞਾ(drag_owner.no_owner == true);
		fallthrough;
	ਹਾਲਤ HTML_DRAG_TEXTAREA_SELECTION:
	ਹਾਲਤ HTML_DRAG_CONTENT_SELECTION:
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.drag.type = CONTENT_DRAG_SELECTION;
		ਰੋਕੋ;
	}
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html, CONTENT_ਸੁਨੇਹਾ_DRAG, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
}

/* Documented in html_internal.h */
ਖਾਲੀ html_ਸੈੱਟ_focus(html_content *html, html_focus_type focus_type,
		ਸੰਘ html_focus_owner focus_owner, bool hide_caret,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਉਚਾਈ, ਅਚਲ ਬਣਤਰ rect *clip)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਪੂਰਨ x_off = 0;
	ਪੂਰਨ y_off = 0;
	ਬਣਤਰ rect cr;
	bool textarea_ਗੁਆਚਾ_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ਪ੍ਰਤਿਜ੍ਞਾ(html != ਨਿਰਾ);

	ਚੋਣ (focus_type) {
	ਹਾਲਤ HTML_FOCUS_SELF:
		ਪ੍ਰਤਿਜ੍ਞਾ(focus_owner.self == true);
		ਜੇ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ਵਾਪਸ;
		ਰੋਕੋ;

	ਹਾਲਤ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ਰੋਕੋ;

	ਹਾਲਤ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ਰੋਕੋ;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ਜੇ (textarea_ਗੁਆਚਾ_focus) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.type = CONTENT_CARET_REMOVE;
	} ਨਹੀਂਤਾਂ ਜੇ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.type = CONTENT_CARET_HIDE;
	} ਨਹੀਂਤਾਂ {
		ਜੇ (clip != ਨਿਰਾ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.type = CONTENT_CARET_ਸੈੱਟ_POS;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.pos.x = x + x_off;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.pos.y = y + y_off;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.pos.ਉਚਾਈ = ਉਚਾਈ;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.caret.pos.clip = (clip == ਨਿਰਾ) ? ਨਿਰਾ : &cr;
	}

	/* Inform of the content's drag status change */
	content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html, CONTENT_ਸੁਨੇਹਾ_CARET, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
}

/* Documented in html_internal.h */
ਖਾਲੀ html_ਸੈੱਟ_selection(html_content *html, html_selection_type selection_type,
		ਸੰਘ html_selection_owner selection_owner, bool read_only)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਬਣਤਰ box *box;
	bool ਬਦਲਿਆ = false;
	bool same_type = html->selection_type == selection_type;

	ਪ੍ਰਤਿਜ੍ਞਾ(html != ਨਿਰਾ);

	ਜੇ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		ਬਦਲਿਆ = true;

	/* Clear any existing selection */
	ਜੇ (html->selection_type != HTML_SELECTION_NONE) {
		ਚੋਣ (html->selection_type) {
		ਹਾਲਤ HTML_SELECTION_SELF:
			ਜੇ (same_type)
				ਰੋਕੋ;
			selection_clear(html->sel, true);
			ਰੋਕੋ;
		ਹਾਲਤ HTML_SELECTION_TEXTAREA:
			ਜੇ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ਰੋਕੋ;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta);
			ਰੋਕੋ;
		ਹਾਲਤ HTML_SELECTION_CONTENT:
			ਜੇ (same_type && html->selection_owner.content ==
					selection_owner.content)
				ਰੋਕੋ;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ਰੋਕੋ;
		ਮੂਲ:
			ਰੋਕੋ;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ਜੇ (!ਬਦਲਿਆ)
		/* Don't need to report lack of change to owner */
		ਵਾਪਸ;

	/* Prepare msg */
	ਚੋਣ (selection_type) {
	ਹਾਲਤ HTML_SELECTION_NONE:
		ਪ੍ਰਤਿਜ੍ਞਾ(selection_owner.none == true);
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.selection.selection = false;
		ਰੋਕੋ;
	ਹਾਲਤ HTML_SELECTION_SELF:
		ਪ੍ਰਤਿਜ੍ਞਾ(selection_owner.none == false);
		fallthrough;
	ਹਾਲਤ HTML_SELECTION_TEXTAREA:
	ਹਾਲਤ HTML_SELECTION_CONTENT:
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.selection.selection = true;
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_ਪ੍ਰਸਾਰਣ((ਬਣਤਰ content *)html, CONTENT_ਸੁਨੇਹਾ_SELECTION,
			&ਸੁਨੇਹਾ_ਅੰਕੜੇ);
}
