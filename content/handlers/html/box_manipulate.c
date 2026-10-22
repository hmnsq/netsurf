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
স্থির পূর্ণসংখ্যা box_talloc_destructor(কাঠামো box *b)
{
	কাঠামো html_scrollbar_তথ্য *তথ্য;

	যদি ((b->flags & STYLE_OWNED) && b->style != ফাঁকা) {
		css_computed_style_destroy(b->style);
		b->style = ফাঁকা;
	}

	যদি (b->styles != ফাঁকা) {
		css_select_results_destroy(b->styles);
		b->styles = ফাঁকা;
	}

	যদি (b->href != ফাঁকা)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	যদি (b->গ্রন্থি != ফাঁকা) {
		dom_গ্রন্থি_unref(b->গ্রন্থি);
	}

	যদি (b->scroll_x != ফাঁকা) {
		তথ্য = scrollbar_get_তথ্য(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		মুক্ত(তথ্য);
	}

	যদি (b->scroll_y != ফাঁকা) {
		তথ্য = scrollbar_get_তথ্য(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		মুক্ত(তথ্য);
	}

	ফেরত 0;
}


/* Exported function documented in html/box.h */
কাঠামো box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ধ্রুবক অক্ষর *target,
	   ধ্রুবক অক্ষর *title,
	   lwc_string *id,
	   শূন্য *context)
{
	unsigned পূর্ণসংখ্যা i;
	কাঠামো box *box;

	box = talloc(context, কাঠামো box);
	যদি (!box) {
		ফেরত 0;
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
	জন্য (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = ফাঁকা;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = ফাঁকা;
	box->দৈর্ঘ্য = 0;
	box->space = 0;
	box->href = (href == ফাঁকা) ? ফাঁকা : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->পরবর্তী = ফাঁকা;
	box->পূর্ব = ফাঁকা;
	box->children = ফাঁকা;
	box->last = ফাঁকা;
	box->parent = ফাঁকা;
	box->inline_end = ফাঁকা;
	box->float_children = ফাঁকা;
	box->float_container = ফাঁকা;
	box->পরবর্তী_float = ফাঁকা;
	box->cached_place_below_level = 0;
	box->list_মান = 1;
	box->list_marker = ফাঁকা;
	box->col = ফাঁকা;
	box->gadget = ফাঁকা;
	box->usemap = ফাঁকা;
	box->id = id;
	box->background = ফাঁকা;
	box->object = ফাঁকা;
	box->object_params = ফাঁকা;
	box->iframe = ফাঁকা;
	box->গ্রন্থি = ফাঁকা;

	ফেরত box;
}


/* Exported function documented in html/box.h */
শূন্য box_add_child(কাঠামো box *parent, কাঠামো box *child)
{
	প্রতিজ্ঞা(parent);
	প্রতিজ্ঞা(child);

	যদি (parent->children != 0) {	/* has children already */
		parent->last->পরবর্তী = child;
		child->পূর্ব = parent->last;
	} অন্যথা {			/* this is the first child */
		parent->children = child;
		child->পূর্ব = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
শূন্য box_ঢোকাও_sibling(কাঠামো box *box, কাঠামো box *new_box)
{
	new_box->parent = box->parent;
	new_box->পূর্ব = box;
	new_box->পরবর্তী = box->পরবর্তী;
	box->পরবর্তী = new_box;
	যদি (new_box->পরবর্তী)
		new_box->পরবর্তী->পূর্ব = new_box;
	অন্যথা যদি (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
শূন্য box_unlink_and_free(কাঠামো box *box)
{
	কাঠামো box *parent = box->parent;
	কাঠামো box *পরবর্তী = box->পরবর্তী;
	কাঠামো box *পূর্ব = box->পূর্ব;

	যদি (parent) {
		যদি (parent->children == box)
			parent->children = পরবর্তী;
		যদি (parent->last == box)
			parent->last = পরবর্তী ? পরবর্তী : পূর্ব;
	}

	যদি (পূর্ব)
		পূর্ব->পরবর্তী = পরবর্তী;
	যদি (পরবর্তী)
		পরবর্তী->পূর্ব = পূর্ব;

	box_free(box);
}


/* Exported function documented in html/box.h */
শূন্য box_free(কাঠামো box *box)
{
	কাঠামো box *child, *পরবর্তী;

	/* free children first */
	জন্য (child = box->children; child; child = পরবর্তী) {
		পরবর্তী = child->পরবর্তী;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
শূন্য box_free_box(কাঠামো box *box)
{
	যদি (!(box->flags & CLONE)) {
		যদি (box->gadget)
			form_free_control(box->gadget);
		যদি (box->scroll_x != ফাঁকা)
			scrollbar_destroy(box->scroll_x);
		যদি (box->scroll_y != ফাঁকা)
			scrollbar_destroy(box->scroll_y);
		যদি (box->styles != ফাঁকা)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(কাঠামো content *c,
		      কাঠামো box *box,
		      bool bottom,
		      bool ডান)
{
	কাঠামো html_scrollbar_তথ্য *তথ্য;
	পূর্ণসংখ্যা visible_width, visible_height;
	পূর্ণসংখ্যা full_width, full_height;
	nserror res;

	যদি (!bottom && box->scroll_x != ফাঁকা) {
		তথ্য = scrollbar_get_তথ্য(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		মুক্ত(তথ্য);
		box->scroll_x = ফাঁকা;
	}

	যদি (!ডান && box->scroll_y != ফাঁকা) {
		তথ্য = scrollbar_get_তথ্য(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		মুক্ত(তথ্য);
		box->scroll_y = ফাঁকা;
	}

	যদি (!bottom && !ডান) {
		ফেরত NSERROR_OK;
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
		যদি (box->scroll_y == ফাঁকা) {
			তথ্য = malloc(আকার(কাঠামো html_scrollbar_তথ্য));
			যদি (তথ্য == ফাঁকা) {
				ফেরত NSERROR_NOMEM;
			}
			তথ্য->c = c;
			তথ্য->box = box;
			res = scrollbar_create(false,
					       visible_height,
					       full_height,
					       visible_height,
					       তথ্য,
					       html_overflow_scroll_callback,
					       &(box->scroll_y));
			যদি (res != NSERROR_OK) {
				ফেরত res;
			}
		} অন্যথা  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	যদি (bottom) {
		যদি (box->scroll_x == ফাঁকা) {
			তথ্য = malloc(আকার(কাঠামো html_scrollbar_তথ্য));
			যদি (তথ্য == ফাঁকা) {
				ফেরত NSERROR_OK;
			}
			তথ্য->c = c;
			তথ্য->box = box;
			res = scrollbar_create(true,
					       visible_width - (ডান ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       তথ্য,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			যদি (res != NSERROR_OK) {
				ফেরত res;
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

	ফেরত NSERROR_OK;
}


