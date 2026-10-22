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
ശൂന്യം box_coords(ഘടന box *box, പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
ശൂന്യം box_bounds(ഘടന box *box, ഘടന rect *r);


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
ഘടന box *box_at_point(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *box, സ്ഥിരം പൂർണ്ണസംഖ്യ x, സ്ഥിരം പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ *box_x, പൂർണ്ണസംഖ്യ *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
ഘടന box *box_find_by_id(ഘടന box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(ഘടന box *box);


/**
 * Print a box tree to a file.
 */
ശൂന്യം box_dump(FILE *stream, ഘടന box *box, unsigned പൂർണ്ണസംഖ്യ depth, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(സ്ഥിരം ഘടന box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(സ്ഥിരം ഘടന box *box);


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
ഘടന box *box_pick_text_box(ഘടന html_content *html, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ dir, പൂർണ്ണസംഖ്യ *dx, പൂർണ്ണസംഖ്യ *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
സ്ഥിതം വരിയിൽ bool box_is_first_child(ഘടന box *b)
{
	തിരികെ (b->parent == ശൂന്യ || b == b->parent->children);
}

സ്ഥിതം വരിയിൽ unsigned box_എണ്ണം_children(സ്ഥിരം ഘടന box *b)
{
	സ്ഥിരം ഘടന box *c = b->children;
	unsigned എണ്ണം = 0;

	ആകുമ്പോൾ (c != ശൂന്യ) {
		എണ്ണം++;
		c = c->next;
	}

	തിരികെ എണ്ണം;
}

#endif
