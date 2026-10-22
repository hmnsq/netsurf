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


#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "netsurf/mouse.h"
#அந்தர்பாவ "desktop/scrollbar.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/interaction.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
நிலையான முழு box_talloc_destructor(கட்டமைப்பு box *b)
{
	கட்டமைப்பு html_scrollbar_data *data;

	எனில் ((b->flags & STYLE_OWNED) && b->style != வெற்று) {
		css_computed_style_destroy(b->style);
		b->style = வெற்று;
	}

	எனில் (b->styles != வெற்று) {
		css_select_results_destroy(b->styles);
		b->styles = வெற்று;
	}

	எனில் (b->href != வெற்று)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	எனில் (b->முனை != வெற்று) {
		dom_முனை_unref(b->முனை);
	}

	எனில் (b->scroll_x != வெற்று) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		விடுவி(data);
	}

	எனில் (b->scroll_y != வெற்று) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		விடுவி(data);
	}

	திரும்பு 0;
}


/* Exported function documented in html/box.h */
கட்டமைப்பு box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   மாறிலி எழுத்து *target,
	   மாறிலி எழுத்து *title,
	   lwc_string *id,
	   வெறுமை *context)
{
	unsigned முழு i;
	கட்டமைப்பு box *box;

	box = talloc(context, கட்டமைப்பு box);
	எனில் (!box) {
		திரும்பு 0;
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
	க்கு (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = வெற்று;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = வெற்று;
	box->length = 0;
	box->space = 0;
	box->href = (href == வெற்று) ? வெற்று : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = வெற்று;
	box->prev = வெற்று;
	box->children = வெற்று;
	box->last = வெற்று;
	box->parent = வெற்று;
	box->inline_end = வெற்று;
	box->float_children = வெற்று;
	box->float_container = வெற்று;
	box->next_float = வெற்று;
	box->cached_place_below_level = 0;
	box->list_மதிப்பு = 1;
	box->list_marker = வெற்று;
	box->col = வெற்று;
	box->gadget = வெற்று;
	box->usemap = வெற்று;
	box->id = id;
	box->background = வெற்று;
	box->object = வெற்று;
	box->object_params = வெற்று;
	box->iframe = வெற்று;
	box->முனை = வெற்று;

	திரும்பு box;
}


/* Exported function documented in html/box.h */
வெறுமை box_add_child(கட்டமைப்பு box *parent, கட்டமைப்பு box *child)
{
	ப்ரதிஜ்ஞா(parent);
	ப்ரதிஜ்ஞா(child);

	எனில் (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} இல்லையெனில் {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
வெறுமை box_செருகு_sibling(கட்டமைப்பு box *box, கட்டமைப்பு box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	எனில் (new_box->next)
		new_box->next->prev = new_box;
	இல்லையெனில் எனில் (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
வெறுமை box_unlink_and_free(கட்டமைப்பு box *box)
{
	கட்டமைப்பு box *parent = box->parent;
	கட்டமைப்பு box *next = box->next;
	கட்டமைப்பு box *prev = box->prev;

	எனில் (parent) {
		எனில் (parent->children == box)
			parent->children = next;
		எனில் (parent->last == box)
			parent->last = next ? next : prev;
	}

	எனில் (prev)
		prev->next = next;
	எனில் (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
வெறுமை box_free(கட்டமைப்பு box *box)
{
	கட்டமைப்பு box *child, *next;

	/* free children first */
	க்கு (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
வெறுமை box_free_box(கட்டமைப்பு box *box)
{
	எனில் (!(box->flags & CLONE)) {
		எனில் (box->gadget)
			form_free_control(box->gadget);
		எனில் (box->scroll_x != வெற்று)
			scrollbar_destroy(box->scroll_x);
		எனில் (box->scroll_y != வெற்று)
			scrollbar_destroy(box->scroll_y);
		எனில் (box->styles != வெற்று)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(கட்டமைப்பு content *c,
		      கட்டமைப்பு box *box,
		      bool bottom,
		      bool வலது)
{
	கட்டமைப்பு html_scrollbar_data *data;
	முழு visible_width, visible_height;
	முழு full_width, full_height;
	nserror res;

	எனில் (!bottom && box->scroll_x != வெற்று) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		விடுவி(data);
		box->scroll_x = வெற்று;
	}

	எனில் (!வலது && box->scroll_y != வெற்று) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		விடுவி(data);
		box->scroll_y = வெற்று;
	}

	எனில் (!bottom && !வலது) {
		திரும்பு NSERROR_OK;
	}

	visible_width = box->width + box->padding[வலது_2] + box->padding[இடது_2];
	visible_height = box->height + box->padding[உச்சி_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[வலது_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[வலது_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	எனில் (வலது) {
		எனில் (box->scroll_y == வெற்று) {
			data = malloc(அளவு(கட்டமைப்பு html_scrollbar_data));
			எனில் (data == வெற்று) {
				திரும்பு NSERROR_NOMEM;
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
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}
		} இல்லையெனில்  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	எனில் (bottom) {
		எனில் (box->scroll_x == வெற்று) {
			data = malloc(அளவு(கட்டமைப்பு html_scrollbar_data));
			எனில் (data == வெற்று) {
				திரும்பு NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (வலது ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}
		} இல்லையெனில் {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(வலது ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	எனில் (வலது && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	திரும்பு NSERROR_OK;
}


