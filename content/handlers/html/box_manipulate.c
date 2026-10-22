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


#ضمن "utils/errors.h"
#ضمن "utils/talloc.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/types.h"
#ضمن "netsurf/mouse.h"
#ضمن "desktop/scrollbar.h"

#ضمن "html/private.h"
#ضمن "html/form_internal.h"
#ضمن "html/interaction.h"
#ضمن "html/box.h"
#ضمن "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_talloc_destructor(بنية box *b)
{
	بنية html_scrollbar_data *data;

	אם ((b->flags & STYLE_OWNED) && b->style != عدم) {
		css_computed_style_destroy(b->style);
		b->style = عدم;
	}

	אם (b->styles != عدم) {
		css_select_results_destroy(b->styles);
		b->styles = عدم;
	}

	אם (b->href != عدم)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	אם (b->node != عدم) {
		dom_node_unref(b->node);
	}

	אם (b->scroll_x != عدم) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		حرر(data);
	}

	אם (b->scroll_y != عدم) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		حرر(data);
	}

	החזר 0;
}


/* Exported function documented in html/box.h */
بنية box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت حرف *target,
	   ثابت حرف *title,
	   lwc_string *id,
	   فراغ *context)
{
	unsigned صحيح i;
	بنية box *box;

	box = talloc(context, بنية box);
	אם (!box) {
		החזר 0;
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
	עבור (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = عدم;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = عدم;
	box->length = 0;
	box->space = 0;
	box->href = (href == عدم) ? عدم : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = عدم;
	box->prev = عدم;
	box->children = عدم;
	box->last = عدم;
	box->parent = عدم;
	box->inline_end = عدم;
	box->float_children = عدم;
	box->float_container = عدم;
	box->next_float = عدم;
	box->cached_place_below_level = 0;
	box->list_value = 1;
	box->list_marker = عدم;
	box->col = عدم;
	box->gadget = عدم;
	box->usemap = عدم;
	box->id = id;
	box->background = عدم;
	box->object = عدم;
	box->object_params = عدم;
	box->iframe = عدم;
	box->node = عدم;

	החזר box;
}


/* Exported function documented in html/box.h */
فراغ box_add_child(بنية box *parent, بنية box *child)
{
	أكد(parent);
	أكد(child);

	אם (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} אחרת {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
فراغ box_insert_sibling(بنية box *box, بنية box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	אם (new_box->next)
		new_box->next->prev = new_box;
	אחרת אם (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
فراغ box_unlink_and_free(بنية box *box)
{
	بنية box *parent = box->parent;
	بنية box *next = box->next;
	بنية box *prev = box->prev;

	אם (parent) {
		אם (parent->children == box)
			parent->children = next;
		אם (parent->last == box)
			parent->last = next ? next : prev;
	}

	אם (prev)
		prev->next = next;
	אם (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
فراغ box_free(بنية box *box)
{
	بنية box *child, *next;

	/* free children first */
	עבור (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
فراغ box_free_box(بنية box *box)
{
	אם (!(box->flags & CLONE)) {
		אם (box->gadget)
			form_free_control(box->gadget);
		אם (box->scroll_x != عدم)
			scrollbar_destroy(box->scroll_x);
		אם (box->scroll_y != عدم)
			scrollbar_destroy(box->scroll_y);
		אם (box->styles != عدم)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(بنية content *c,
		      بنية box *box,
		      bool bottom,
		      bool right)
{
	بنية html_scrollbar_data *data;
	صحيح visible_width, visible_height;
	صحيح full_width, full_height;
	nserror res;

	אם (!bottom && box->scroll_x != عدم) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		حرر(data);
		box->scroll_x = عدم;
	}

	אם (!right && box->scroll_y != عدم) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		حرر(data);
		box->scroll_y = عدم;
	}

	אם (!bottom && !right) {
		החזר NSERROR_OK;
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

	אם (right) {
		אם (box->scroll_y == عدم) {
			data = malloc(حجم(بنية html_scrollbar_data));
			אם (data == عدم) {
				החזר NSERROR_NOMEM;
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
			אם (res != NSERROR_OK) {
				החזר res;
			}
		} אחרת  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	אם (bottom) {
		אם (box->scroll_x == عدم) {
			data = malloc(حجم(بنية html_scrollbar_data));
			אם (data == عدم) {
				החזר NSERROR_OK;
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
			אם (res != NSERROR_OK) {
				החזר res;
			}
		} אחרת {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(right ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	אם (right && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	החזר NSERROR_OK;
}


