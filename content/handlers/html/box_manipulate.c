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


#شامل "utils/errors.h"
#شامل "utils/talloc.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/types.h"
#شامل "netsurf/mouse.h"
#شامل "desktop/scrollbar.h"

#شامل "html/private.h"
#شامل "html/form_internal.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ایستا صحیح box_talloc_destructor(ساختار box *b)
{
	ساختار html_scrollbar_data *data;

	اگر ((b->flags & STYLE_OWNED) && b->style != پوچ) {
		css_computed_style_destroy(b->style);
		b->style = پوچ;
	}

	اگر (b->styles != پوچ) {
		css_select_results_destroy(b->styles);
		b->styles = پوچ;
	}

	اگر (b->href != پوچ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	اگر (b->گره != پوچ) {
		dom_گره_unref(b->گره);
	}

	اگر (b->scroll_x != پوچ) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		آزاد(data);
	}

	اگر (b->scroll_y != پوچ) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		آزاد(data);
	}

	بازگشت 0;
}


/* Exported function documented in html/box.h */
ساختار box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت نویسه *target,
	   ثابت نویسه *title,
	   lwc_string *id,
	   تهی *context)
{
	unsigned صحیح i;
	ساختار box *box;

	box = talloc(context, ساختار box);
	اگر (!box) {
		بازگشت 0;
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
	برای (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = پوچ;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = پوچ;
	box->length = 0;
	box->space = 0;
	box->href = (href == پوچ) ? پوچ : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = پوچ;
	box->prev = پوچ;
	box->children = پوچ;
	box->last = پوچ;
	box->parent = پوچ;
	box->inline_end = پوچ;
	box->float_children = پوچ;
	box->float_container = پوچ;
	box->next_float = پوچ;
	box->cached_place_below_level = 0;
	box->list_مقدار = 1;
	box->list_marker = پوچ;
	box->col = پوچ;
	box->gadget = پوچ;
	box->usemap = پوچ;
	box->id = id;
	box->background = پوچ;
	box->object = پوچ;
	box->object_params = پوچ;
	box->iframe = پوچ;
	box->گره = پوچ;

	بازگشت box;
}


/* Exported function documented in html/box.h */
تهی box_add_child(ساختار box *parent, ساختار box *child)
{
	أكد(parent);
	أكد(child);

	اگر (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} وگرنه {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
تهی box_درج_sibling(ساختار box *box, ساختار box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	اگر (new_box->next)
		new_box->next->prev = new_box;
	وگرنه اگر (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
تهی box_unlink_and_free(ساختار box *box)
{
	ساختار box *parent = box->parent;
	ساختار box *next = box->next;
	ساختار box *prev = box->prev;

	اگر (parent) {
		اگر (parent->children == box)
			parent->children = next;
		اگر (parent->last == box)
			parent->last = next ? next : prev;
	}

	اگر (prev)
		prev->next = next;
	اگر (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
تهی box_free(ساختار box *box)
{
	ساختار box *child, *next;

	/* free children first */
	برای (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
تهی box_free_box(ساختار box *box)
{
	اگر (!(box->flags & CLONE)) {
		اگر (box->gadget)
			form_free_control(box->gadget);
		اگر (box->scroll_x != پوچ)
			scrollbar_destroy(box->scroll_x);
		اگر (box->scroll_y != پوچ)
			scrollbar_destroy(box->scroll_y);
		اگر (box->styles != پوچ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ساختار content *c,
		      ساختار box *box,
		      bool bottom,
		      bool راست)
{
	ساختار html_scrollbar_data *data;
	صحیح visible_width, visible_height;
	صحیح full_width, full_height;
	nserror res;

	اگر (!bottom && box->scroll_x != پوچ) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		آزاد(data);
		box->scroll_x = پوچ;
	}

	اگر (!راست && box->scroll_y != پوچ) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		آزاد(data);
		box->scroll_y = پوچ;
	}

	اگر (!bottom && !راست) {
		بازگشت NSERROR_OK;
	}

	visible_width = box->width + box->padding[راست_2] + box->padding[چپ_2];
	visible_height = box->height + box->padding[بالاترین_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[راست_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[راست_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	اگر (راست) {
		اگر (box->scroll_y == پوچ) {
			data = malloc(اندازه(ساختار html_scrollbar_data));
			اگر (data == پوچ) {
				بازگشت NSERROR_NOMEM;
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
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}
		} وگرنه  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	اگر (bottom) {
		اگر (box->scroll_x == پوچ) {
			data = malloc(اندازه(ساختار html_scrollbar_data));
			اگر (data == پوچ) {
				بازگشت NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (راست ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}
		} وگرنه {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(راست ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	اگر (راست && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	بازگشت NSERROR_OK;
}


