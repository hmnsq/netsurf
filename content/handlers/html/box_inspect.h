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

#నిర్వచితం_కాకపోతే NETSURF_HTML_BOX_INSPECT_H
#నిర్వచించు NETSURF_HTML_BOX_INSPECT_H

/**
 * Find the absolute coordinates of a box.
 *
 * \param  box  the box to calculate coordinates of
 * \param  x    updated to x coordinate
 * \param  y    updated to y coordinate
 */
శూన్య box_coords(నిర్మాణం box *box, పూర్ణాంకం *x, పూర్ణాంకం *y);


/**
 * Find the bounds of a box.
 *
 * \param  box  the box to calculate bounds of
 * \param  r    receives bounds
 */
శూన్య box_bounds(నిర్మాణం box *box, నిర్మాణం rect *r);


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
నిర్మాణం box *box_at_బిందువు(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *box, స్థిరరాశి పూర్ణాంకం x, స్థిరరాశి పూర్ణాంకం y, పూర్ణాంకం *box_x, పూర్ణాంకం *box_y);


/**
 * Find a box based upon its id attribute.
 *
 * \param  box  box tree to search
 * \param  id   id to look for
 * \return  the box or 0 if not found
 */
నిర్మాణం box *box_కనుగొను_by_id(నిర్మాణం box *box, lwc_string *id);


/**
 * Determine if a box is visible when the tree is rendered.
 *
 * \param  box  box to check
 * \return  true iff the box is rendered
 */
bool box_visible(నిర్మాణం box *box);


/**
 * Print a box tree to a file.
 */
శూన్య box_dump(FILE *ప్రవాహం, నిర్మాణం box *box, గుర్తులేని పూర్ణాంకం లోతు, bool style);


/**
 * Determine if a box has a vertical scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a vertical scrollbar
 */
bool box_vscrollbar_present(స్థిరరాశి నిర్మాణం box *box);


/**
 * Determine if a box has a horizontal scrollbar.
 *
 * \param  box  scrolling box
 * \return the box has a horizontal scrollbar
 */
bool box_hscrollbar_present(స్థిరరాశి నిర్మాణం box *box);


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
నిర్మాణం box *box_pick_పాఠ్యం_box(నిర్మాణం html_content *html, పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం dir, పూర్ణాంకం *dx, పూర్ణాంకం *dy);


/**
 * Check if layout box is a first child.
 *
 * \param[in] b  Box to check.
 * \return true iff box is first child.
 */
స్థిర అంతఃవరుస bool box_is_మొదటి_బిడ్డ(నిర్మాణం box *b)
{
	తిరిగి (b->మాతృ == ఖాళీ || b == b->మాతృ->children);
}

స్థిర అంతఃవరుస గుర్తులేని box_లెక్క_children(స్థిరరాశి నిర్మాణం box *b)
{
	స్థిరరాశి నిర్మాణం box *c = b->children;
	గుర్తులేని లెక్క = 0;

	అయితే (c != ఖాళీ) {
		లెక్క++;
		c = c->తదుపరి;
	}

	తిరిగి లెక్క;
}

#అయితే_ముగింపు
