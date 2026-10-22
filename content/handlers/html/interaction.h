/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
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
 * HTML content user interaction handling
 */

#اگر_غیر_تعریف_شدہ NETSURF_HTML_INTERACTION_H
#تعریف_کرو NETSURF_HTML_INTERACTION_H

#شامل "desktop/search.h" /* search flags enum */

/**
 * Context for scrollbar
 */
ساخت html_scrollbar_ڈیٹا {
	ساخت content *c;
	ساخت box *box;
};

/**
 * Handle mouse tracking (including drags) in an HTML content window.
 *
 * \param  c	  content of type html
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
nserror html_mouse_track(ساخت content *c, ساخت browser_کھڑکی *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);


/**
 * Handle mouse clicks and movements in an HTML content window.
 *
 * This function handles both hovering and clicking. It is important that the
 * code path is identical (except that hovering doesn't carry out the action),
 * so that the status bar reflects exactly what will happen. Having separate
 * code paths opens the possibility that an attacker will make the status bar
 * show some harmless action where clicking will be harmful.
 *
 * \param c content of type html
 * \param bw browser window
 * \param mouse state of mouse buttons and modifier keys
 * \param x x coordinate of mouse
 * \param y y coordinate of mouse
 * \return NSERROR_OK or appropriate error code.
 */
nserror html_mouse_action(ساخت content *c, ساخت browser_کھڑکی *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);


bool html_keypress(ساخت content *c, uint32_t کلید);


خالی html_حد_سے_تجاوز_scroll_callback(خالی *client_ڈیٹا,
		ساخت scrollbar_پیغام_ڈیٹا *scrollbar_ڈیٹا);


خالی html_تلاش(ساخت content *c, خالی *context,
		تلاش_flags_t flags, ثابت حرف *string);


خالی html_تلاش_clear(ساخت content *c);


/**
 * Set our drag status, and inform whatever owns the content
 *
 * \param html		HTML content
 * \param drag_type	Type of drag
 * \param drag_owner	What owns the drag
 * \param rect		Pointer movement bounds
 */
خالی html_سیٹ_drag_type(html_content *html, html_drag_type drag_type,
		اتحاد html_drag_owner drag_owner, ثابت ساخت rect *rect);


/**
 * Set our selection status, and inform whatever owns the content
 *
 * \param html			HTML content
 * \param selection_type	Type of selection
 * \param selection_owner	What owns the selection
 * \param read_only		True iff selection is read only
 */
خالی html_سیٹ_selection(html_content *html, html_selection_type selection_type,
		اتحاد html_selection_owner selection_owner, bool read_only);


/**
 * Set our input focus, and inform whatever owns the content
 *
 * \param html			HTML content
 * \param focus_type		Type of input focus
 * \param focus_owner		What owns the focus
 * \param hide_caret		True iff caret to be hidden
 * \param x			Carret x-coord rel to owner
 * \param y			Carret y-coord rel to owner
 * \param height		Carret height
 * \param clip			Carret clip rect
 */
خالی html_سیٹ_focus(html_content *html, html_focus_type focus_type,
		اتحاد html_focus_owner focus_owner, bool hide_caret,
		صحیح x, صحیح y, صحیح اونچائی, ثابت ساخت rect *clip);


#اگر_ختم
