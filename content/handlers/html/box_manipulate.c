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


#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/interaction.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_talloc_destructor(ସଂରଚନା box *b)
{
	ସଂରଚନା html_scrollbar_data *data;

	ଯଦି ((b->flags & STYLE_OWNED) && b->style != ଖାଲି) {
		css_computed_style_destroy(b->style);
		b->style = ଖାଲି;
	}

	ଯଦି (b->styles != ଖାଲି) {
		css_select_results_destroy(b->styles);
		b->styles = ଖାଲି;
	}

	ଯଦି (b->href != ଖାଲି)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ଯଦି (b->ଗଣ୍ଠି != ଖାଲି) {
		dom_ଗଣ୍ଠି_unref(b->ଗଣ୍ଠି);
	}

	ଯଦି (b->scroll_x != ଖାଲି) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		ମୁକ୍ତ(data);
	}

	ଯଦି (b->scroll_y != ଖାଲି) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		ମୁକ୍ତ(data);
	}

	ଫେରନ୍ତୁ 0;
}


/* Exported function documented in html/box.h */
ସଂରଚନା box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ଅଚଳ ଅକ୍ଷର *target,
	   ଅଚଳ ଅକ୍ଷର *title,
	   lwc_string *id,
	   ଶୂନ୍ୟ *context)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା box *box;

	box = talloc(context, ସଂରଚନା box);
	ଯଦି (!box) {
		ଫେରନ୍ତୁ 0;
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
	ପାଇଁ (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = ଖାଲି;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = ଖାଲି;
	box->length = 0;
	box->space = 0;
	box->href = (href == ଖାଲି) ? ଖାଲି : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = ଖାଲି;
	box->prev = ଖାଲି;
	box->children = ଖାଲି;
	box->last = ଖାଲି;
	box->parent = ଖାଲି;
	box->inline_end = ଖାଲି;
	box->float_children = ଖାଲି;
	box->float_container = ଖାଲି;
	box->next_float = ଖାଲି;
	box->cached_place_below_level = 0;
	box->list_ମୂଲ୍ୟ = 1;
	box->list_marker = ଖାଲି;
	box->col = ଖାଲି;
	box->gadget = ଖାଲି;
	box->usemap = ଖାଲି;
	box->id = id;
	box->background = ଖାଲି;
	box->object = ଖାଲି;
	box->object_params = ଖାଲି;
	box->iframe = ଖାଲି;
	box->ଗଣ୍ଠି = ଖାଲି;

	ଫେରନ୍ତୁ box;
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_add_child(ସଂରଚନା box *parent, ସଂରଚନା box *child)
{
	ପ୍ରତିଜ୍ଞା(parent);
	ପ୍ରତିଜ୍ଞା(child);

	ଯଦି (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} ଅନ୍ୟଥା {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_ଭର୍ତ୍ତି_sibling(ସଂରଚନା box *box, ସଂରଚନା box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	ଯଦି (new_box->next)
		new_box->next->prev = new_box;
	ଅନ୍ୟଥା ଯଦି (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_unlink_and_free(ସଂରଚନା box *box)
{
	ସଂରଚନା box *parent = box->parent;
	ସଂରଚନା box *next = box->next;
	ସଂରଚନା box *prev = box->prev;

	ଯଦି (parent) {
		ଯଦି (parent->children == box)
			parent->children = next;
		ଯଦି (parent->last == box)
			parent->last = next ? next : prev;
	}

	ଯଦି (prev)
		prev->next = next;
	ଯଦି (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_free(ସଂରଚନା box *box)
{
	ସଂରଚନା box *child, *next;

	/* free children first */
	ପାଇଁ (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_free_box(ସଂରଚନା box *box)
{
	ଯଦି (!(box->flags & CLONE)) {
		ଯଦି (box->gadget)
			form_free_control(box->gadget);
		ଯଦି (box->scroll_x != ଖାଲି)
			scrollbar_destroy(box->scroll_x);
		ଯଦି (box->scroll_y != ଖାଲି)
			scrollbar_destroy(box->scroll_y);
		ଯଦି (box->styles != ଖାଲି)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ସଂରଚନା content *c,
		      ସଂରଚନା box *box,
		      bool bottom,
		      bool ଡାହାଣ)
{
	ସଂରଚନା html_scrollbar_data *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା visible_width, visible_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା full_width, full_height;
	nserror res;

	ଯଦି (!bottom && box->scroll_x != ଖାଲି) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		ମୁକ୍ତ(data);
		box->scroll_x = ଖାଲି;
	}

	ଯଦି (!ଡାହାଣ && box->scroll_y != ଖାଲି) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		ମୁକ୍ତ(data);
		box->scroll_y = ଖାଲି;
	}

	ଯଦି (!bottom && !ଡାହାଣ) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	visible_width = box->width + box->padding[ଡାହାଣ_2] + box->padding[ବାମ_2];
	visible_height = box->height + box->padding[ଶୀର୍ଷ_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[ଡାହାଣ_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[ଡାହାଣ_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	ଯଦି (ଡାହାଣ) {
		ଯଦି (box->scroll_y == ଖାଲି) {
			data = malloc(ଆକାର(ସଂରଚନା html_scrollbar_data));
			ଯଦି (data == ଖାଲି) {
				ଫେରନ୍ତୁ NSERROR_NOMEM;
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
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
		} ଅନ୍ୟଥା  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	ଯଦି (bottom) {
		ଯଦି (box->scroll_x == ଖାଲି) {
			data = malloc(ଆକାର(ସଂରଚନା html_scrollbar_data));
			ଯଦି (data == ଖାଲି) {
				ଫେରନ୍ତୁ NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (ଡାହାଣ ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
		} ଅନ୍ୟଥା {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(ଡାହାଣ ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	ଯଦି (ଡାହାଣ && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


