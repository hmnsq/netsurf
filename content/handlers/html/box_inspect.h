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
शून्य box_coords(संरचना box *box, पूर्णाङ्क *x, पूर्णाङ्क *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
शून्य box_bounds(संरचना box *box, संरचना rect *r);


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
संरचना box *box_at_point(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *box, ध्रुव पूर्णाङ्क x, ध्रुव पूर्णाङ्क y, पूर्णाङ्क *box_x, पूर्णाङ्क *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
संरचना box *box_find_by_id(संरचना box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(संरचना box *box);


/**
 * Print a box tree to a file.
 */
शून्य box_dump(FILE *stream, संरचना box *box, unsigned पूर्णाङ्क depth, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(ध्रुव संरचना box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(ध्रुव संरचना box *box);


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
संरचना box *box_pick_text_box(संरचना html_content *html, पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क dir, पूर्णाङ्क *dx, पूर्णाङ्क *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
स्थिर अन्तःस्थ bool box_is_first_child(संरचना box *b)
{
	प्रत्यागम (b->parent == रिक्त || b == b->parent->children);
}

स्थिर अन्तःस्थ unsigned box_गणनम्_children(ध्रुव संरचना box *b)
{
	ध्रुव संरचना box *c = b->children;
	unsigned गणनम् = 0;

	जाव (c != रिक्त) {
		गणनम्++;
		c = c->next;
	}

	प्रत्यागम गणनम्;
}

#endif
