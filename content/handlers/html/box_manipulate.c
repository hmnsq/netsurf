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


#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "netsurf/mouse.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/interaction.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ box_talloc_destructor(ഘടന box *b)
{
	ഘടന html_scrollbar_data *data;

	എങ്കിൽ ((b->flags & STYLE_OWNED) && b->style != ശൂന്യ) {
		css_computed_style_destroy(b->style);
		b->style = ശൂന്യ;
	}

	എങ്കിൽ (b->styles != ശൂന്യ) {
		css_select_results_destroy(b->styles);
		b->styles = ശൂന്യ;
	}

	എങ്കിൽ (b->href != ശൂന്യ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	എങ്കിൽ (b->കണ്ണി != ശൂന്യ) {
		dom_കണ്ണി_unref(b->കണ്ണി);
	}

	എങ്കിൽ (b->scroll_x != ശൂന്യ) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		സ്വതന്ത്രം(data);
	}

	എങ്കിൽ (b->scroll_y != ശൂന്യ) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		സ്വതന്ത്രം(data);
	}

	തിരികെ 0;
}


/* Exported function documented in html/box.h */
ഘടന box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   സ്ഥിരം അക്ഷരം *target,
	   സ്ഥിരം അക്ഷരം *title,
	   lwc_string *id,
	   ശൂന്യം *context)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന box *box;

	box = talloc(context, ഘടന box);
	എങ്കിൽ (!box) {
		തിരികെ 0;
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
	വേണ്ടി (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = ശൂന്യ;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = ശൂന്യ;
	box->length = 0;
	box->space = 0;
	box->href = (href == ശൂന്യ) ? ശൂന്യ : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = ശൂന്യ;
	box->prev = ശൂന്യ;
	box->children = ശൂന്യ;
	box->last = ശൂന്യ;
	box->parent = ശൂന്യ;
	box->inline_end = ശൂന്യ;
	box->float_children = ശൂന്യ;
	box->float_container = ശൂന്യ;
	box->next_float = ശൂന്യ;
	box->cached_place_below_level = 0;
	box->list_മൂല്യം = 1;
	box->list_marker = ശൂന്യ;
	box->col = ശൂന്യ;
	box->gadget = ശൂന്യ;
	box->usemap = ശൂന്യ;
	box->id = id;
	box->background = ശൂന്യ;
	box->object = ശൂന്യ;
	box->object_params = ശൂന്യ;
	box->iframe = ശൂന്യ;
	box->കണ്ണി = ശൂന്യ;

	തിരികെ box;
}


/* Exported function documented in html/box.h */
ശൂന്യം box_add_child(ഘടന box *parent, ഘടന box *child)
{
	പ്രതിജ്ഞാ(parent);
	പ്രതിജ്ഞാ(child);

	എങ്കിൽ (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} അല്ലെങ്കിൽ {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
ശൂന്യം box_തിരുകുക_sibling(ഘടന box *box, ഘടന box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	എങ്കിൽ (new_box->next)
		new_box->next->prev = new_box;
	അല്ലെങ്കിൽ എങ്കിൽ (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
ശൂന്യം box_unlink_and_free(ഘടന box *box)
{
	ഘടന box *parent = box->parent;
	ഘടന box *next = box->next;
	ഘടന box *prev = box->prev;

	എങ്കിൽ (parent) {
		എങ്കിൽ (parent->children == box)
			parent->children = next;
		എങ്കിൽ (parent->last == box)
			parent->last = next ? next : prev;
	}

	എങ്കിൽ (prev)
		prev->next = next;
	എങ്കിൽ (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
ശൂന്യം box_free(ഘടന box *box)
{
	ഘടന box *child, *next;

	/* free children first */
	വേണ്ടി (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
ശൂന്യം box_free_box(ഘടന box *box)
{
	എങ്കിൽ (!(box->flags & CLONE)) {
		എങ്കിൽ (box->gadget)
			form_free_control(box->gadget);
		എങ്കിൽ (box->scroll_x != ശൂന്യ)
			scrollbar_destroy(box->scroll_x);
		എങ്കിൽ (box->scroll_y != ശൂന്യ)
			scrollbar_destroy(box->scroll_y);
		എങ്കിൽ (box->styles != ശൂന്യ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ഘടന content *c,
		      ഘടന box *box,
		      bool bottom,
		      bool വലത്)
{
	ഘടന html_scrollbar_data *data;
	പൂർണ്ണസംഖ്യ visible_width, visible_height;
	പൂർണ്ണസംഖ്യ full_width, full_height;
	nserror res;

	എങ്കിൽ (!bottom && box->scroll_x != ശൂന്യ) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		സ്വതന്ത്രം(data);
		box->scroll_x = ശൂന്യ;
	}

	എങ്കിൽ (!വലത് && box->scroll_y != ശൂന്യ) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		സ്വതന്ത്രം(data);
		box->scroll_y = ശൂന്യ;
	}

	എങ്കിൽ (!bottom && !വലത്) {
		തിരികെ NSERROR_OK;
	}

	visible_width = box->width + box->padding[വലത്_2] + box->padding[ഇടത്_2];
	visible_height = box->height + box->padding[മുകൾ_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[വലത്_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[വലത്_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	എങ്കിൽ (വലത്) {
		എങ്കിൽ (box->scroll_y == ശൂന്യ) {
			data = malloc(വലുപ്പം(ഘടന html_scrollbar_data));
			എങ്കിൽ (data == ശൂന്യ) {
				തിരികെ NSERROR_NOMEM;
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}
		} അല്ലെങ്കിൽ  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	എങ്കിൽ (bottom) {
		എങ്കിൽ (box->scroll_x == ശൂന്യ) {
			data = malloc(വലുപ്പം(ഘടന html_scrollbar_data));
			എങ്കിൽ (data == ശൂന്യ) {
				തിരികെ NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (വലത് ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}
		} അല്ലെങ്കിൽ {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(വലത് ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	എങ്കിൽ (വലത് && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	തിരികെ NSERROR_OK;
}


