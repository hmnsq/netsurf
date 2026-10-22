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
void box_coords(sa_nracanaa box *box, ཧྲིལ *x, ཧྲིལ *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
void box_bounds(sa_nracanaa box *box, sa_nracanaa rect *r);


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
sa_nracanaa box *box_at_point(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *box, const ཧྲིལ x, const ཧྲིལ y, ཧྲིལ *box_x, ཧྲིལ *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
sa_nracanaa box *box_find_by_id(sa_nracanaa box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(sa_nracanaa box *box);


/**
 * Print a box tree to a file.
 */
void box_dump(FILE *stream, sa_nracanaa box *box, unsigned ཧྲིལ depth, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(const sa_nracanaa box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(const sa_nracanaa box *box);


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
sa_nracanaa box *box_pick_text_box(sa_nracanaa html_content *html, ཧྲིལ x, ཧྲིལ y, ཧྲིལ dir, ཧྲིལ *dx, ཧྲིལ *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
static anta_hstha bool box_is_first_child(sa_nracanaa box *b)
{
	ལོག (b->parent == NULL || b == b->parent->children);
}

static anta_hstha unsigned box_count_children(const sa_nracanaa box *b)
{
	const sa_nracanaa box *c = b->children;
	unsigned count = 0;

	སྐབས (c != NULL) {
		count++;
		c = c->next;
	}

	ལོག count;
}

#endif
