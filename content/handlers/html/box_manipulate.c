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


#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "netsurf/mouse.h"
#অন্তর্ভাব "desktop/scrollbar.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/form_internal.h"
#অন্তর্ভাব "html/interaction.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
স্থির পূর্ণাঙ্ক box_talloc_destructor(সংরচনা box *b)
{
	সংরচনা html_scrollbar_data *data;

	যদি ((b->flags & STYLE_OWNED) && b->style != রিক্ত) {
		css_computed_style_destroy(b->style);
		b->style = রিক্ত;
	}

	যদি (b->styles != রিক্ত) {
		css_select_results_destroy(b->styles);
		b->styles = রিক্ত;
	}

	যদি (b->href != রিক্ত)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	যদি (b->গ্রন্থি != রিক্ত) {
		dom_গ্রন্থি_unref(b->গ্রন্থি);
	}

	যদি (b->scroll_x != রিক্ত) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		মুক্ত(data);
	}

	যদি (b->scroll_y != রিক্ত) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		মুক্ত(data);
	}

	প্রত্যাগম 0;
}


/* Exported function documented in html/box.h */
সংরচনা box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ধ্রুব বর্ণ *target,
	   ধ্রুব বর্ণ *title,
	   lwc_string *id,
	   শূন্য *context)
{
	unsigned পূর্ণাঙ্ক i;
	সংরচনা box *box;

	box = talloc(context, সংরচনা box);
	যদি (!box) {
		প্রত্যাগম 0;
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
	প্রতি (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = রিক্ত;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = রিক্ত;
	box->length = 0;
	box->space = 0;
	box->href = (href == রিক্ত) ? রিক্ত : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = রিক্ত;
	box->prev = রিক্ত;
	box->children = রিক্ত;
	box->last = রিক্ত;
	box->parent = রিক্ত;
	box->inline_end = রিক্ত;
	box->float_children = রিক্ত;
	box->float_container = রিক্ত;
	box->next_float = রিক্ত;
	box->cached_place_below_level = 0;
	box->list_মান = 1;
	box->list_marker = রিক্ত;
	box->col = রিক্ত;
	box->gadget = রিক্ত;
	box->usemap = রিক্ত;
	box->id = id;
	box->background = রিক্ত;
	box->object = রিক্ত;
	box->object_params = রিক্ত;
	box->iframe = রিক্ত;
	box->গ্রন্থি = রিক্ত;

	প্রত্যাগম box;
}


/* Exported function documented in html/box.h */
শূন্য box_add_child(সংরচনা box *parent, সংরচনা box *child)
{
	প্রতিজ্ঞা(parent);
	প্রতিজ্ঞা(child);

	যদি (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} অন্যথা {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
শূন্য box_ঢোকাও_sibling(সংরচনা box *box, সংরচনা box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	যদি (new_box->next)
		new_box->next->prev = new_box;
	অন্যথা যদি (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
শূন্য box_unlink_and_free(সংরচনা box *box)
{
	সংরচনা box *parent = box->parent;
	সংরচনা box *next = box->next;
	সংরচনা box *prev = box->prev;

	যদি (parent) {
		যদি (parent->children == box)
			parent->children = next;
		যদি (parent->last == box)
			parent->last = next ? next : prev;
	}

	যদি (prev)
		prev->next = next;
	যদি (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
শূন্য box_free(সংরচনা box *box)
{
	সংরচনা box *child, *next;

	/* free children first */
	প্রতি (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
শূন্য box_free_box(সংরচনা box *box)
{
	যদি (!(box->flags & CLONE)) {
		যদি (box->gadget)
			form_free_control(box->gadget);
		যদি (box->scroll_x != রিক্ত)
			scrollbar_destroy(box->scroll_x);
		যদি (box->scroll_y != রিক্ত)
			scrollbar_destroy(box->scroll_y);
		যদি (box->styles != রিক্ত)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(সংরচনা content *c,
		      সংরচনা box *box,
		      bool bottom,
		      bool ডান)
{
	সংরচনা html_scrollbar_data *data;
	পূর্ণাঙ্ক visible_width, visible_height;
	পূর্ণাঙ্ক full_width, full_height;
	nserror res;

	যদি (!bottom && box->scroll_x != রিক্ত) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		মুক্ত(data);
		box->scroll_x = রিক্ত;
	}

	যদি (!ডান && box->scroll_y != রিক্ত) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		মুক্ত(data);
		box->scroll_y = রিক্ত;
	}

	যদি (!bottom && !ডান) {
		প্রত্যাগম NSERROR_OK;
	}

	visible_width = box->width + box->padding[ডান_2] + box->padding[বাম_2];
	visible_height = box->height + box->padding[শীর্ষ_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[ডান_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[ডান_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	যদি (ডান) {
		যদি (box->scroll_y == রিক্ত) {
			data = malloc(পরিমাণ(সংরচনা html_scrollbar_data));
			যদি (data == রিক্ত) {
				প্রত্যাগম NSERROR_NOMEM;
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
			যদি (res != NSERROR_OK) {
				প্রত্যাগম res;
			}
		} অন্যথা  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	যদি (bottom) {
		যদি (box->scroll_x == রিক্ত) {
			data = malloc(পরিমাণ(সংরচনা html_scrollbar_data));
			যদি (data == রিক্ত) {
				প্রত্যাগম NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (ডান ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			যদি (res != NSERROR_OK) {
				প্রত্যাগম res;
			}
		} অন্যথা {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(ডান ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	যদি (ডান && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	প্রত্যাগম NSERROR_OK;
}


