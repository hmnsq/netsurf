/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
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
 * Box tree manipulation interface.
 */

#अगर_अघोषित NETSURF_HTML_BOX_MANIPULATE_H
#घोषणा NETSURF_HTML_BOX_MANIPULATE_H


/**
 * Create a box tree node.
 *
 * \param  styles       selection results for the box, or NULL
 * \param  style        computed style for the box (not copied), or 0
 * \param  style_owned  whether style is owned by this box
 * \param  href         href for the box (copied), or 0
 * \param  target       target for the box (not copied), or 0
 * \param  title        title for the box (not copied), or 0
 * \param  id           id for the box (not copied), or 0
 * \param  context      context for allocations
 * \return  allocated and initialised box, or 0 on memory exhaustion
 *
 * styles is always owned by the box, if it is set.
 * style is only owned by the box in the case of implied boxes.
 */
काष्ठा box * box_create(css_select_results *styles, css_computed_style *style, bool style_owned, काष्ठा nsurl *href, स्थिर अक्षर *target, स्थिर अक्षर *title, lwc_string *id, व्योम *context);


/**
 * Add a child to a box tree node.
 *
 * \param parent box giving birth
 * \param child box to link as last child of parent
 */
व्योम box_add_child(काष्ठा box *parent, काष्ठा box *child);


/**
 * Insert a new box as a sibling to a box in a tree.
 *
 * \param box box already in tree
 * \param new_box box to link into tree as next sibling
 */
व्योम box_जोड़ो_sibling(काष्ठा box *box, काष्ठा box *new_box);


/**
 * Unlink a box from the box tree and then free it recursively.
 *
 * \param box box to unlink and free recursively.
 */
व्योम box_unlink_and_free(काष्ठा box *box);


/**
 * Free a box tree recursively.
 *
 * \param  box  box to free recursively
 *
 * The box and all its children is freed.
 */
व्योम box_free(काष्ठा box *box);


/**
 * Free the data in a single box structure.
 *
 * \param box box to free
 */
व्योम box_free_box(काष्ठा box *box);


/**
 * Applies the given scroll setup to a box. This includes scroll
 * creation/deletion as well as scroll dimension updates.
 *
 * \param c		content in which the box is located
 * \param box		the box to handle the scrolls for
 * \param bottom	whether the horizontal scrollbar should be present
 * \param right		whether the vertical scrollbar should be present
 * \return		true on success false otherwise
 */
nserror box_handle_scrollbars(काष्ठा content *c, काष्ठा box *box,
		bool bottom, bool दायाँ);


#पूर्ण_अगर
