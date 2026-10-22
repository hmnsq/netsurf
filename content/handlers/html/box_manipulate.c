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


#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "netsurf/types.h"
#અન્તર્ભાવ "netsurf/mouse.h"
#અન્તર્ભાવ "desktop/scrollbar.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/interaction.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
સ્થિર પૂર્ણાંક box_talloc_destructor(સંરચના box *b)
{
	સંરચના html_scrollbar_data *data;

	જો ((b->flags & STYLE_OWNED) && b->style != ખાલી) {
		css_computed_style_destroy(b->style);
		b->style = ખાલી;
	}

	જો (b->styles != ખાલી) {
		css_select_results_destroy(b->styles);
		b->styles = ખાલી;
	}

	જો (b->href != ખાલી)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	જો (b->ગાંઠ != ખાલી) {
		dom_ગાંઠ_unref(b->ગાંઠ);
	}

	જો (b->scroll_x != ખાલી) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		મુક્ત(data);
	}

	જો (b->scroll_y != ખાલી) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		મુક્ત(data);
	}

	પાછા 0;
}


/* Exported function documented in html/box.h */
સંરચના box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   અચલ અક્ષર *target,
	   અચલ અક્ષર *title,
	   lwc_string *id,
	   શૂન્ય *context)
{
	unsigned પૂર્ણાંક i;
	સંરચના box *box;

	box = talloc(context, સંરચના box);
	જો (!box) {
		પાછા 0;
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
	માટે (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = ખાલી;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = ખાલી;
	box->length = 0;
	box->space = 0;
	box->href = (href == ખાલી) ? ખાલી : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = ખાલી;
	box->prev = ખાલી;
	box->children = ખાલી;
	box->last = ખાલી;
	box->parent = ખાલી;
	box->inline_end = ખાલી;
	box->float_children = ખાલી;
	box->float_container = ખાલી;
	box->next_float = ખાલી;
	box->cached_place_below_level = 0;
	box->list_કિંમત = 1;
	box->list_marker = ખાલી;
	box->col = ખાલી;
	box->gadget = ખાલી;
	box->usemap = ખાલી;
	box->id = id;
	box->background = ખાલી;
	box->object = ખાલી;
	box->object_params = ખાલી;
	box->iframe = ખાલી;
	box->ગાંઠ = ખાલી;

	પાછા box;
}


/* Exported function documented in html/box.h */
શૂન્ય box_add_child(સંરચના box *parent, સંરચના box *child)
{
	પ્રતિજ્ઞા(parent);
	પ્રતિજ્ઞા(child);

	જો (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} નહીંતર {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
શૂન્ય box_ઉમેરો_sibling(સંરચના box *box, સંરચના box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	જો (new_box->next)
		new_box->next->prev = new_box;
	નહીંતર જો (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
શૂન્ય box_unlink_and_free(સંરચના box *box)
{
	સંરચના box *parent = box->parent;
	સંરચના box *next = box->next;
	સંરચના box *prev = box->prev;

	જો (parent) {
		જો (parent->children == box)
			parent->children = next;
		જો (parent->last == box)
			parent->last = next ? next : prev;
	}

	જો (prev)
		prev->next = next;
	જો (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
શૂન્ય box_free(સંરચના box *box)
{
	સંરચના box *child, *next;

	/* free children first */
	માટે (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
શૂન્ય box_free_box(સંરચના box *box)
{
	જો (!(box->flags & CLONE)) {
		જો (box->gadget)
			form_free_control(box->gadget);
		જો (box->scroll_x != ખાલી)
			scrollbar_destroy(box->scroll_x);
		જો (box->scroll_y != ખાલી)
			scrollbar_destroy(box->scroll_y);
		જો (box->styles != ખાલી)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(સંરચના content *c,
		      સંરચના box *box,
		      bool bottom,
		      bool જમણું)
{
	સંરચના html_scrollbar_data *data;
	પૂર્ણાંક visible_width, visible_height;
	પૂર્ણાંક full_width, full_height;
	nserror res;

	જો (!bottom && box->scroll_x != ખાલી) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		મુક્ત(data);
		box->scroll_x = ખાલી;
	}

	જો (!જમણું && box->scroll_y != ખાલી) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		મુક્ત(data);
		box->scroll_y = ખાલી;
	}

	જો (!bottom && !જમણું) {
		પાછા NSERROR_OK;
	}

	visible_width = box->width + box->padding[જમણું_2] + box->padding[ડાબું_2];
	visible_height = box->height + box->padding[ટોચ_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[જમણું_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[જમણું_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	જો (જમણું) {
		જો (box->scroll_y == ખાલી) {
			data = malloc(કદ(સંરચના html_scrollbar_data));
			જો (data == ખાલી) {
				પાછા NSERROR_NOMEM;
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
			જો (res != NSERROR_OK) {
				પાછા res;
			}
		} નહીંતર  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	જો (bottom) {
		જો (box->scroll_x == ખાલી) {
			data = malloc(કદ(સંરચના html_scrollbar_data));
			જો (data == ખાલી) {
				પાછા NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (જમણું ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			જો (res != NSERROR_OK) {
				પાછા res;
			}
		} નહીંતર {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(જમણું ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	જો (જમણું && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	પાછા NSERROR_OK;
}


