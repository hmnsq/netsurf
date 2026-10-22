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


#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ box_talloc_destructor(ಸಂರಚನಾ box *b)
{
	ಸಂರಚನಾ html_scrollbar_data *data;

	ಯದಿ ((b->flags & STYLE_OWNED) && b->style != ರಿಕ್ತ) {
		css_computed_style_destroy(b->style);
		b->style = ರಿಕ್ತ;
	}

	ಯದಿ (b->styles != ರಿಕ್ತ) {
		css_select_results_destroy(b->styles);
		b->styles = ರಿಕ್ತ;
	}

	ಯದಿ (b->href != ರಿಕ್ತ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ಯದಿ (b->ಗಂಟು != ರಿಕ್ತ) {
		dom_ಗಂಟು_unref(b->ಗಂಟು);
	}

	ಯದಿ (b->scroll_x != ರಿಕ್ತ) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		ಮುಕ್ತ(data);
	}

	ಯದಿ (b->scroll_y != ರಿಕ್ತ) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		ಮುಕ್ತ(data);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}


/* Exported function documented in html/box.h */
ಸಂರಚನಾ box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ಧ್ರುವ ವರ್ಣ *target,
	   ಧ್ರುವ ವರ್ಣ *title,
	   lwc_string *id,
	   ಶೂನ್ಯ *context)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ box *box;

	box = talloc(context, ಸಂರಚನಾ box);
	ಯದಿ (!box) {
		ಪ್ರತ್ಯಾಗಮ 0;
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
	ಪ್ರತಿ (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = ರಿಕ್ತ;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = ರಿಕ್ತ;
	box->length = 0;
	box->space = 0;
	box->href = (href == ರಿಕ್ತ) ? ರಿಕ್ತ : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = ರಿಕ್ತ;
	box->prev = ರಿಕ್ತ;
	box->children = ರಿಕ್ತ;
	box->last = ರಿಕ್ತ;
	box->parent = ರಿಕ್ತ;
	box->inline_end = ರಿಕ್ತ;
	box->float_children = ರಿಕ್ತ;
	box->float_container = ರಿಕ್ತ;
	box->next_float = ರಿಕ್ತ;
	box->cached_place_below_level = 0;
	box->list_ಮೌಲ್ಯ = 1;
	box->list_marker = ರಿಕ್ತ;
	box->col = ರಿಕ್ತ;
	box->gadget = ರಿಕ್ತ;
	box->usemap = ರಿಕ್ತ;
	box->id = id;
	box->background = ರಿಕ್ತ;
	box->object = ರಿಕ್ತ;
	box->object_params = ರಿಕ್ತ;
	box->iframe = ರಿಕ್ತ;
	box->ಗಂಟು = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ box;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_add_child(ಸಂರಚನಾ box *parent, ಸಂರಚನಾ box *child)
{
	ಪ್ರತಿಜ್ಞಾ(parent);
	ಪ್ರತಿಜ್ಞಾ(child);

	ಯದಿ (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} ಅನ್ಯಥಾ {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_ಸೇರಿಸು_sibling(ಸಂರಚನಾ box *box, ಸಂರಚನಾ box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	ಯದಿ (new_box->next)
		new_box->next->prev = new_box;
	ಅನ್ಯಥಾ ಯದಿ (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_unlink_and_free(ಸಂರಚನಾ box *box)
{
	ಸಂರಚನಾ box *parent = box->parent;
	ಸಂರಚನಾ box *next = box->next;
	ಸಂರಚನಾ box *prev = box->prev;

	ಯದಿ (parent) {
		ಯದಿ (parent->children == box)
			parent->children = next;
		ಯದಿ (parent->last == box)
			parent->last = next ? next : prev;
	}

	ಯದಿ (prev)
		prev->next = next;
	ಯದಿ (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_free(ಸಂರಚನಾ box *box)
{
	ಸಂರಚನಾ box *child, *next;

	/* free children first */
	ಪ್ರತಿ (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_free_box(ಸಂರಚನಾ box *box)
{
	ಯದಿ (!(box->flags & CLONE)) {
		ಯದಿ (box->gadget)
			form_free_control(box->gadget);
		ಯದಿ (box->scroll_x != ರಿಕ್ತ)
			scrollbar_destroy(box->scroll_x);
		ಯದಿ (box->scroll_y != ರಿಕ್ತ)
			scrollbar_destroy(box->scroll_y);
		ಯದಿ (box->styles != ರಿಕ್ತ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ಸಂರಚನಾ content *c,
		      ಸಂರಚನಾ box *box,
		      bool bottom,
		      bool ಬಲ)
{
	ಸಂರಚನಾ html_scrollbar_data *data;
	ಪೂರ್ಣಾಙ್ಕ visible_width, visible_height;
	ಪೂರ್ಣಾಙ್ಕ full_width, full_height;
	nserror res;

	ಯದಿ (!bottom && box->scroll_x != ರಿಕ್ತ) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		ಮುಕ್ತ(data);
		box->scroll_x = ರಿಕ್ತ;
	}

	ಯದಿ (!ಬಲ && box->scroll_y != ರಿಕ್ತ) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		ಮುಕ್ತ(data);
		box->scroll_y = ರಿಕ್ತ;
	}

	ಯದಿ (!bottom && !ಬಲ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	visible_width = box->width + box->padding[ಬಲ_2] + box->padding[ಎಡ_2];
	visible_height = box->height + box->padding[ತುದಿ_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[ಬಲ_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[ಬಲ_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	ಯದಿ (ಬಲ) {
		ಯದಿ (box->scroll_y == ರಿಕ್ತ) {
			data = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ html_scrollbar_data));
			ಯದಿ (data == ರಿಕ್ತ) {
				ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}
		} ಅನ್ಯಥಾ  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	ಯದಿ (bottom) {
		ಯದಿ (box->scroll_x == ರಿಕ್ತ) {
			data = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ html_scrollbar_data));
			ಯದಿ (data == ರಿಕ್ತ) {
				ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (ಬಲ ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}
		} ಅನ್ಯಥಾ {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(ಬಲ ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	ಯದಿ (ಬಲ && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


