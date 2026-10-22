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
 * HTML Box tree inspection interface.
 */

#ifndef NETSURF_HTML_BOX_INSPECT_H
#define NETSURF_HTML_BOX_INSPECT_H

/**
 * Find the absolute coordinates of a box.
 *
 * \param  box  the box to calculate coordinates of
 * \param  x    updated to x coordinate
 * \param  y    updated to y coordinate
 */
শূন্য box_coords(সংরচনা box *box, পূর্ণাঙ্ক *x, পূর্ণাঙ্ক *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
শূন্য box_bounds(সংরচনা box *box, সংরচনা rect *r);


/**
 * Find the boxes at a point.
 *
 * \param  unit_len_ctx  CSS length conversion context for document.
 * \param  box      box to search children of
 * \param  x        point to find, in global document coordinates
 * \param  y        point to find, in global document coordinates
 * \param  box_x    position of box, in global document coordinates, updated
 *                  to position of returned box, if any
 * \param  box_y    position of box, in global document coordinates, updated
 *                  to position of returned box, if any
 * \return  box at given point, or 0 if none found
 *
 * To find all the boxes in the hierarchy at a certain point, use code like
 * this:
 * \code
 *	struct box *box = top_of_document_to_search;
 *	int box_x = 0, box_y = 0;
 *
 *	while ((box = box_at_point(unit_len_ctx, box, x, y, &box_x, &box_y))) {
 *		// process box
 *	}
 * \endcode
 */
সংরচনা box *box_at_point(ধ্রুব css_unit_ctx *unit_len_ctx, সংরচনা box *box, ধ্রুব পূর্ণাঙ্ক x, ধ্রুব পূর্ণাঙ্ক y, পূর্ণাঙ্ক *box_x, পূর্ণাঙ্ক *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
সংরচনা box *box_find_by_id(সংরচনা box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(সংরচনা box *box);


/**
 * Print a box tree to a file.
 */
শূন্য box_dump(FILE *stream, সংরচনা box *box, unsigned পূর্ণাঙ্ক depth, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(ধ্রুব সংরচনা box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(ধ্রুব সংরচনা box *box);


/**
 * Peform pick text on browser window contents to locate the box under
 * the mouse pointer, or nearest in the given direction if the pointer is
 * not over a text box.
 *
 * \param html	an HTML content
 * \param x	coordinate of mouse
 * \param y	coordinate of mouse
 * \param dir	direction to search (-1 = above-left, +1 = below-right)
 * \param dx	receives x ordinate of mouse relative to text box
 * \param dy	receives y ordinate of mouse relative to text box
 */
সংরচনা box *box_pick_text_box(সংরচনা html_content *html, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক dir, পূর্ণাঙ্ক *dx, পূর্ণাঙ্ক *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
স্থির অন্তঃস্থ bool box_is_first_child(সংরচনা box *b)
{
	প্রত্যাগম (b->parent == রিক্ত || b == b->parent->children);
}

স্থির অন্তঃস্থ unsigned box_গুনতি_children(ধ্রুব সংরচনা box *b)
{
	ধ্রুব সংরচনা box *c = b->children;
	unsigned গুনতি = 0;

	যাবত্ (c != রিক্ত) {
		গুনতি++;
		c = c->next;
	}

	প্রত্যাগম গুনতি;
}

#endif
