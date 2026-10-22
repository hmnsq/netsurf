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

#నిర్వచితం_కాకపోతే NETSURF_HTML_INTERACTION_H
#నిర్వచించు NETSURF_HTML_INTERACTION_H

#అన్తర్భావ "desktop/search.h" /* search flags enum */

/**
 * Context for scrollbar
 */
నిర్మాణం html_scrollbar_దత్తాంశం {
	నిర్మాణం content *c;
	నిర్మాణం box *box;
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
nserror html_mouse_track(నిర్మాణం content *c, నిర్మాణం browser_కిటికీ *bw,
			browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);


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
nserror html_mouse_action(నిర్మాణం content *c, నిర్మాణం browser_కిటికీ *bw,
			browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);


bool html_keypress(నిర్మాణం content *c, uint32_t కీలకం);


శూన్య html_అధికప్రవాహం_scroll_callback(శూన్య *client_దత్తాంశం,
		నిర్మాణం scrollbar_సందేశం_దత్తాంశం *scrollbar_దత్తాంశం);


శూన్య html_శోధన(నిర్మాణం content *c, శూన్య *context,
		శోధన_flags_t flags, స్థిరరాశి అక్షరం *string);


శూన్య html_శోధన_clear(నిర్మాణం content *c);


/**
 * Set our drag status, and inform whatever owns the content
 *
 * \param html		HTML content
 * \param drag_type	Type of drag
 * \param drag_owner	What owns the drag
 * \param rect		Pointer movement bounds
 */
శూన్య html_సమితి_drag_type(html_content *html, html_drag_type drag_type,
		సంఘం html_drag_owner drag_owner, స్థిరరాశి నిర్మాణం rect *rect);


/**
 * Set our selection status, and inform whatever owns the content
 *
 * \param html			HTML content
 * \param selection_type	Type of selection
 * \param selection_owner	What owns the selection
 * \param read_only		True iff selection is read only
 */
శూన్య html_సమితి_selection(html_content *html, html_selection_type selection_type,
		సంఘం html_selection_owner selection_owner, bool read_only);


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
శూన్య html_సమితి_focus(html_content *html, html_focus_type focus_type,
		సంఘం html_focus_owner focus_owner, bool hide_caret,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం ఎత్తు, స్థిరరాశి నిర్మాణం rect *clip);


#అయితే_ముగింపు
