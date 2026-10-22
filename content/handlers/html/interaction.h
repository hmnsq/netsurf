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

#अगर_अघोषित NETSURF_HTML_INTERACTION_H
#घोषणा NETSURF_HTML_INTERACTION_H

#समावेश "desktop/search.h" /* search flags enum */

/**
 * Context for scrollbar
 */
काष्ठा html_scrollbar_तथ्य {
	काष्ठा content *c;
	काष्ठा box *box;
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
nserror html_mouse_track(काष्ठा content *c, काष्ठा browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);


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
nserror html_mouse_action(काष्ठा content *c, काष्ठा browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);


bool html_keypress(काष्ठा content *c, uint32_t कुंजी);


व्योम html_overflow_scroll_callback(व्योम *client_तथ्य,
		काष्ठा scrollbar_संदेश_तथ्य *scrollbar_तथ्य);


व्योम html_खोज(काष्ठा content *c, व्योम *context,
		खोज_flags_t flags, स्थिर अक्षर *string);


व्योम html_खोज_clear(काष्ठा content *c);


/**
 * Set our drag status, and inform whatever owns the content
 *
 * \param html		HTML content
 * \param drag_type	Type of drag
 * \param drag_owner	What owns the drag
 * \param rect		Pointer movement bounds
 */
व्योम html_set_drag_type(html_content *html, html_drag_type drag_type,
		जोड़ html_drag_owner drag_owner, स्थिर काष्ठा rect *rect);


/**
 * Set our selection status, and inform whatever owns the content
 *
 * \param html			HTML content
 * \param selection_type	Type of selection
 * \param selection_owner	What owns the selection
 * \param read_only		True iff selection is read only
 */
व्योम html_set_selection(html_content *html, html_selection_type selection_type,
		जोड़ html_selection_owner selection_owner, bool read_only);


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
व्योम html_set_focus(html_content *html, html_focus_type focus_type,
		जोड़ html_focus_owner focus_owner, bool hide_caret,
		पूर्णांक x, पूर्णांक y, पूर्णांक height, स्थिर काष्ठा rect *clip);


#पूर्ण_अगर
