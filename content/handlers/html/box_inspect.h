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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_BOX_INSPECT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_BOX_INSPECT_H

/**
 * Find the absolute coordinates of a box.
 *
 * \param  box  the box to calculate coordinates of
 * \param  x    updated to x coordinate
 * \param  y    updated to y coordinate
 */
ਖਾਲੀ box_coords(ਬਣਤਰ box *box, ਪੂਰਨ *x, ਪੂਰਨ *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
ਖਾਲੀ box_bounds(ਬਣਤਰ box *box, ਬਣਤਰ rect *r);


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
ਬਣਤਰ box *box_at_ਬਿੰਦੂ(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *box, ਅਚਲ ਪੂਰਨ x, ਅਚਲ ਪੂਰਨ y, ਪੂਰਨ *box_x, ਪੂਰਨ *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
ਬਣਤਰ box *box_ਲੱਭੋ_by_id(ਬਣਤਰ box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(ਬਣਤਰ box *box);


/**
 * Print a box tree to a file.
 */
ਖਾਲੀ box_dump(FILE *ਧਾਰਾ, ਬਣਤਰ box *box, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਡੂੰਘਾਈ, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(ਅਚਲ ਬਣਤਰ box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(ਅਚਲ ਬਣਤਰ box *box);


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
ਬਣਤਰ box *box_pick_ਪਾਠ_box(ਬਣਤਰ html_content *html, ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ dir, ਪੂਰਨ *dx, ਪੂਰਨ *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool box_is_ਪਹਿਲਾ_ਬੱਚਾ(ਬਣਤਰ box *b)
{
	ਵਾਪਸ (b->ਮਾਪਾ == ਨਿਰਾ || b == b->ਮਾਪਾ->children);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ box_ਗਿਣਤੀ_children(ਅਚਲ ਬਣਤਰ box *b)
{
	ਅਚਲ ਬਣਤਰ box *c = b->children;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = 0;

	ਜਦੋਂ (c != ਨਿਰਾ) {
		ਗਿਣਤੀ_2++;
		c = c->ਅਗਲਾ;
	}

	ਵਾਪਸ ਗਿਣਤੀ_2;
}

#ਜੇ_ਅੰਤ
