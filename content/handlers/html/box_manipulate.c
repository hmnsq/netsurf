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


#include "utils/errors.h"
#include "utils/talloc.h"
#include "utils/nsurl.h"
#include "netsurf/types.h"
#include "netsurf/mouse.h"
#include "desktop/scrollbar.h"

#include "html/private.h"
#include "html/form_internal.h"
#include "html/interaction.h"
#include "html/box.h"
#include "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static puur_naa_nka box_talloc_destructor(sa_nracanaa box *b)
{
	sa_nracanaa html_scrollbar_data *data;

	ᰀᰧ ((b->flags & STYLE_OWNED) && b->style != NULL) {
		css_computed_style_destroy(b->style);
		b->style = NULL;
	}

	ᰀᰧ (b->styles != NULL) {
		css_select_results_destroy(b->styles);
		b->styles = NULL;
	}

	ᰀᰧ (b->href != NULL)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ᰀᰧ (b->node != NULL) {
		dom_node_unref(b->node);
	}

	ᰀᰧ (b->scroll_x != NULL) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		free(data);
	}

	ᰀᰧ (b->scroll_y != NULL) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		free(data);
	}

	ᰀᰧᰌ 0;
}


/* Exported function documented in html/box.h */
sa_nracanaa box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   const var_na *target,
	   const var_na *title,
	   lwc_string *id,
	   void *context)
{
	unsigned puur_naa_nka i;
	sa_nracanaa box *box;

	box = talloc(context, sa_nracanaa box);
	ᰀᰧ (!box) {
		ᰀᰧᰌ 0;
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
	for (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = NULL;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = NULL;
	box->length = 0;
	box->space = 0;
	box->href = (href == NULL) ? NULL : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = NULL;
	box->prev = NULL;
	box->children = NULL;
	box->last = NULL;
	box->parent = NULL;
	box->inline_end = NULL;
	box->float_children = NULL;
	box->float_container = NULL;
	box->next_float = NULL;
	box->cached_place_below_level = 0;
	box->list_value = 1;
	box->list_marker = NULL;
	box->col = NULL;
	box->gadget = NULL;
	box->usemap = NULL;
	box->id = id;
	box->background = NULL;
	box->object = NULL;
	box->object_params = NULL;
	box->iframe = NULL;
	box->node = NULL;

	ᰀᰧᰌ box;
}


/* Exported function documented in html/box.h */
void box_add_child(sa_nracanaa box *parent, sa_nracanaa box *child)
{
	assert(parent);
	assert(child);

	ᰀᰧ (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} ᰀᰧᰀ {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
void box_insert_sibling(sa_nracanaa box *box, sa_nracanaa box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	ᰀᰧ (new_box->next)
		new_box->next->prev = new_box;
	ᰀᰧᰀ ᰀᰧ (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
void box_unlink_and_free(sa_nracanaa box *box)
{
	sa_nracanaa box *parent = box->parent;
	sa_nracanaa box *next = box->next;
	sa_nracanaa box *prev = box->prev;

	ᰀᰧ (parent) {
		ᰀᰧ (parent->children == box)
			parent->children = next;
		ᰀᰧ (parent->last == box)
			parent->last = next ? next : prev;
	}

	ᰀᰧ (prev)
		prev->next = next;
	ᰀᰧ (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
void box_free(sa_nracanaa box *box)
{
	sa_nracanaa box *child, *next;

	/* free children first */
	for (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
void box_free_box(sa_nracanaa box *box)
{
	ᰀᰧ (!(box->flags & CLONE)) {
		ᰀᰧ (box->gadget)
			form_free_control(box->gadget);
		ᰀᰧ (box->scroll_x != NULL)
			scrollbar_destroy(box->scroll_x);
		ᰀᰧ (box->scroll_y != NULL)
			scrollbar_destroy(box->scroll_y);
		ᰀᰧ (box->styles != NULL)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(sa_nracanaa content *c,
		      sa_nracanaa box *box,
		      bool bottom,
		      bool right)
{
	sa_nracanaa html_scrollbar_data *data;
	puur_naa_nka visible_width, visible_height;
	puur_naa_nka full_width, full_height;
	nserror res;

	ᰀᰧ (!bottom && box->scroll_x != NULL) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		free(data);
		box->scroll_x = NULL;
	}

	ᰀᰧ (!right && box->scroll_y != NULL) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		free(data);
		box->scroll_y = NULL;
	}

	ᰀᰧ (!bottom && !right) {
		ᰀᰧᰌ NSERROR_OK;
	}

	visible_width = box->width + box->padding[RIGHT] + box->padding[LEFT];
	visible_height = box->height + box->padding[TOP] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[RIGHT].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[RIGHT] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	ᰀᰧ (right) {
		ᰀᰧ (box->scroll_y == NULL) {
			data = malloc(parimaa_na(sa_nracanaa html_scrollbar_data));
			ᰀᰧ (data == NULL) {
				ᰀᰧᰌ NSERROR_NOMEM;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(false,
					       visible_height,
					       full_height,
					       visible_height,
					       data,
					       html_overflow_scroll_callback,
					       &(box->scroll_y));
			ᰀᰧ (res != NSERROR_OK) {
				ᰀᰧᰌ res;
			}
		} ᰀᰧᰀ  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	ᰀᰧ (bottom) {
		ᰀᰧ (box->scroll_x == NULL) {
			data = malloc(parimaa_na(sa_nracanaa html_scrollbar_data));
			ᰀᰧ (data == NULL) {
				ᰀᰧᰌ NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (right ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			ᰀᰧ (res != NSERROR_OK) {
				ᰀᰧᰌ res;
			}
		} ᰀᰧᰀ {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(right ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	ᰀᰧ (right && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	ᰀᰧᰌ NSERROR_OK;
}


