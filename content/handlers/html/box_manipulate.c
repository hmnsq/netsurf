/*
 * Copyright 2005-2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
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
 * implementation of box tree manipulation.
 */


#समावेश "utils/errors.h"
#समावेश "utils/talloc.h"
#समावेश "utils/nsurl.h"
#समावेश "netsurf/types.h"
#समावेश "netsurf/mouse.h"
#समावेश "desktop/scrollbar.h"

#समावेश "html/private.h"
#समावेश "html/form_internal.h"
#समावेश "html/interaction.h"
#समावेश "html/box.h"
#समावेश "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
अटल पूर्णांक box_talloc_destructor(काष्ठा box *b)
{
	काष्ठा html_scrollbar_तथ्य *तथ्य;

	अगर ((b->flags & STYLE_OWNED) && b->style != शून्य) {
		css_computed_style_destroy(b->style);
		b->style = शून्य;
	}

	अगर (b->styles != शून्य) {
		css_select_results_destroy(b->styles);
		b->styles = शून्य;
	}

	अगर (b->href != शून्य)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	अगर (b->गाँठ != शून्य) {
		dom_गाँठ_unref(b->गाँठ);
	}

	अगर (b->scroll_x != शून्य) {
		तथ्य = scrollbar_get_तथ्य(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		मुक्त(तथ्य);
	}

	अगर (b->scroll_y != शून्य) {
		तथ्य = scrollbar_get_तथ्य(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		मुक्त(तथ्य);
	}

	वापस 0;
}


/* Exported function documented in html/box.h */
काष्ठा box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   स्थिर अक्षर *target,
	   स्थिर अक्षर *title,
	   lwc_string *id,
	   व्योम *context)
{
	अचिन्हित पूर्णांक i;
	काष्ठा box *box;

	box = talloc(context, काष्ठा box);
	अगर (!box) {
		वापस 0;
	}

	talloc_set_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->width = UNKNOWN_WIDTH;
	box->height = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	क्रम (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = शून्य;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = शून्य;
	box->लंबाई = 0;
	box->space = 0;
	box->href = (href == शून्य) ? शून्य : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->अग्र = शून्य;
	box->पूर्व = शून्य;
	box->children = शून्य;
	box->last = शून्य;
	box->parent = शून्य;
	box->inline_end = शून्य;
	box->float_children = शून्य;
	box->float_container = शून्य;
	box->अग्र_float = शून्य;
	box->cached_place_below_level = 0;
	box->list_मूल्य = 1;
	box->list_marker = शून्य;
	box->col = शून्य;
	box->gadget = शून्य;
	box->usemap = शून्य;
	box->id = id;
	box->background = शून्य;
	box->object = शून्य;
	box->object_params = शून्य;
	box->iframe = शून्य;
	box->गाँठ = शून्य;

	वापस box;
}


/* Exported function documented in html/box.h */
व्योम box_add_child(काष्ठा box *parent, काष्ठा box *child)
{
	निश्चित(parent);
	निश्चित(child);

	अगर (parent->children != 0) {	/* has children already */
		parent->last->अग्र = child;
		child->पूर्व = parent->last;
	} अन्यथा {			/* this is the first child */
		parent->children = child;
		child->पूर्व = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
व्योम box_जोड़ो_sibling(काष्ठा box *box, काष्ठा box *new_box)
{
	new_box->parent = box->parent;
	new_box->पूर्व = box;
	new_box->अग्र = box->अग्र;
	box->अग्र = new_box;
	अगर (new_box->अग्र)
		new_box->अग्र->पूर्व = new_box;
	अन्यथा अगर (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
व्योम box_unlink_and_free(काष्ठा box *box)
{
	काष्ठा box *parent = box->parent;
	काष्ठा box *अग्र = box->अग्र;
	काष्ठा box *पूर्व = box->पूर्व;

	अगर (parent) {
		अगर (parent->children == box)
			parent->children = अग्र;
		अगर (parent->last == box)
			parent->last = अग्र ? अग्र : पूर्व;
	}

	अगर (पूर्व)
		पूर्व->अग्र = अग्र;
	अगर (अग्र)
		अग्र->पूर्व = पूर्व;

	box_free(box);
}


/* Exported function documented in html/box.h */
व्योम box_free(काष्ठा box *box)
{
	काष्ठा box *child, *अग्र;

	/* free children first */
	क्रम (child = box->children; child; child = अग्र) {
		अग्र = child->अग्र;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
व्योम box_free_box(काष्ठा box *box)
{
	अगर (!(box->flags & CLONE)) {
		अगर (box->gadget)
			form_free_control(box->gadget);
		अगर (box->scroll_x != शून्य)
			scrollbar_destroy(box->scroll_x);
		अगर (box->scroll_y != शून्य)
			scrollbar_destroy(box->scroll_y);
		अगर (box->styles != शून्य)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(काष्ठा content *c,
		      काष्ठा box *box,
		      bool bottom,
		      bool दायाँ)
{
	काष्ठा html_scrollbar_तथ्य *तथ्य;
	पूर्णांक visible_width, visible_height;
	पूर्णांक full_width, full_height;
	nserror res;

	अगर (!bottom && box->scroll_x != शून्य) {
		तथ्य = scrollbar_get_तथ्य(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		मुक्त(तथ्य);
		box->scroll_x = शून्य;
	}

	अगर (!दायाँ && box->scroll_y != शून्य) {
		तथ्य = scrollbar_get_तथ्य(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		मुक्त(तथ्य);
		box->scroll_y = शून्य;
	}

	अगर (!bottom && !दायाँ) {
		वापस NSERROR_OK;
	}

	visible_width = box->width + box->padding[दायाँ_2] + box->padding[बायाँ_2];
	visible_height = box->height + box->padding[शीर्ष_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[दायाँ_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[दायाँ_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	अगर (दायाँ) {
		अगर (box->scroll_y == शून्य) {
			तथ्य = दो_स्मृति(माप(काष्ठा html_scrollbar_तथ्य));
			अगर (तथ्य == शून्य) {
				वापस NSERROR_NOMEM;
			}
			तथ्य->c = c;
			तथ्य->box = box;
			res = scrollbar_create(false,
					       visible_height,
					       full_height,
					       visible_height,
					       तथ्य,
					       html_overflow_scroll_callback,
					       &(box->scroll_y));
			अगर (res != NSERROR_OK) {
				वापस res;
			}
		} अन्यथा  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	अगर (bottom) {
		अगर (box->scroll_x == शून्य) {
			तथ्य = दो_स्मृति(माप(काष्ठा html_scrollbar_तथ्य));
			अगर (तथ्य == शून्य) {
				वापस NSERROR_OK;
			}
			तथ्य->c = c;
			तथ्य->box = box;
			res = scrollbar_create(true,
					       visible_width - (दायाँ ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       तथ्य,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			अगर (res != NSERROR_OK) {
				वापस res;
			}
		} अन्यथा {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(दायाँ ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	अगर (दायाँ && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	वापस NSERROR_OK;
}


