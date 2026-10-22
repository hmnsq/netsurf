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


#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "netsurf/mouse.h"
#अन्तर्भाव "desktop/scrollbar.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/interaction.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_talloc_destructor(संरचना box *b)
{
	संरचना html_scrollbar_data *data;

	सचे ((b->flags & STYLE_OWNED) && b->style != रिक्त) {
		css_computed_style_destroy(b->style);
		b->style = रिक्त;
	}

	सचे (b->styles != रिक्त) {
		css_select_results_destroy(b->styles);
		b->styles = रिक्त;
	}

	सचे (b->href != रिक्त)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	सचे (b->ग्रन्थिः != रिक्त) {
		dom_ग्रन्थिः_unref(b->ग्रन्थिः);
	}

	सचे (b->scroll_x != रिक्त) {
		data = scrollbar_get_data(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		मुक्त(data);
	}

	सचे (b->scroll_y != रिक्त) {
		data = scrollbar_get_data(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		मुक्त(data);
	}

	पटिनिवत्त 0;
}


/* Exported function documented in html/box.h */
संरचना box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ध्रुव वर्ण *target,
	   ध्रुव वर्ण *title,
	   lwc_string *id,
	   शून्य *context)
{
	unsigned पूर्णाङ्क i;
	संरचना box *box;

	box = talloc(context, संरचना box);
	सचे (!box) {
		पटिनिवत्त 0;
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
	अत्थाय (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].width = 0;
	box->scroll_x = box->scroll_y = रिक्त;
	box->min_width = 0;
	box->max_width = UNKNOWN_MAX_WIDTH;
	box->byte_offset = 0;
	box->text = रिक्त;
	box->length = 0;
	box->space = 0;
	box->href = (href == रिक्त) ? रिक्त : nsurl_ref(href);
	box->target = target;
	box->title = title;
	box->columns = 1;
	box->rows = 1;
	box->start_column = 0;
	box->next = रिक्त;
	box->prev = रिक्त;
	box->children = रिक्त;
	box->last = रिक्त;
	box->parent = रिक्त;
	box->inline_end = रिक्त;
	box->float_children = रिक्त;
	box->float_container = रिक्त;
	box->next_float = रिक्त;
	box->cached_place_below_level = 0;
	box->list_मानम् = 1;
	box->list_marker = रिक्त;
	box->col = रिक्त;
	box->gadget = रिक्त;
	box->usemap = रिक्त;
	box->id = id;
	box->background = रिक्त;
	box->object = रिक्त;
	box->object_params = रिक्त;
	box->iframe = रिक्त;
	box->ग्रन्थिः = रिक्त;

	पटिनिवत्त box;
}


/* Exported function documented in html/box.h */
शून्य box_add_child(संरचना box *parent, संरचना box *child)
{
	प्रतिज्ञा(parent);
	प्रतिज्ञा(child);

	सचे (parent->children != 0) {	/* has children already */
		parent->last->next = child;
		child->prev = parent->last;
	} अञ्ञथा {			/* this is the first child */
		parent->children = child;
		child->prev = 0;
	}

	parent->last = child;
	child->parent = parent;
}


/* Exported function documented in html/box.h */
शून्य box_निवेशय_sibling(संरचना box *box, संरचना box *new_box)
{
	new_box->parent = box->parent;
	new_box->prev = box;
	new_box->next = box->next;
	box->next = new_box;
	सचे (new_box->next)
		new_box->next->prev = new_box;
	अञ्ञथा सचे (new_box->parent)
		new_box->parent->last = new_box;
}


/* Exported function documented in html/box.h */
शून्य box_unlink_and_free(संरचना box *box)
{
	संरचना box *parent = box->parent;
	संरचना box *next = box->next;
	संरचना box *prev = box->prev;

	सचे (parent) {
		सचे (parent->children == box)
			parent->children = next;
		सचे (parent->last == box)
			parent->last = next ? next : prev;
	}

	सचे (prev)
		prev->next = next;
	सचे (next)
		next->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
शून्य box_free(संरचना box *box)
{
	संरचना box *child, *next;

	/* free children first */
	अत्थाय (child = box->children; child; child = next) {
		next = child->next;
		box_free(child);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
शून्य box_free_box(संरचना box *box)
{
	सचे (!(box->flags & CLONE)) {
		सचे (box->gadget)
			form_free_control(box->gadget);
		सचे (box->scroll_x != रिक्त)
			scrollbar_destroy(box->scroll_x);
		सचे (box->scroll_y != रिक्त)
			scrollbar_destroy(box->scroll_y);
		सचे (box->styles != रिक्त)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(संरचना content *c,
		      संरचना box *box,
		      bool bottom,
		      bool दक्षिणः)
{
	संरचना html_scrollbar_data *data;
	पूर्णाङ्क visible_width, visible_height;
	पूर्णाङ्क full_width, full_height;
	nserror res;

	सचे (!bottom && box->scroll_x != रिक्त) {
		data = scrollbar_get_data(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		मुक्त(data);
		box->scroll_x = रिक्त;
	}

	सचे (!दक्षिणः && box->scroll_y != रिक्त) {
		data = scrollbar_get_data(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		मुक्त(data);
		box->scroll_y = रिक्त;
	}

	सचे (!bottom && !दक्षिणः) {
		पटिनिवत्त NSERROR_OK;
	}

	visible_width = box->width + box->padding[दक्षिणः_2] + box->padding[वामः_2];
	visible_height = box->height + box->padding[शिखरम्_2] + box->padding[BOTTOM];

	full_width = ((box->descendant_x1 - box->border[दक्षिणः_2].width) >
			visible_width) ?
			box->descendant_x1 + box->padding[दक्षिणः_2] :
			visible_width;
	full_height = ((box->descendant_y1 - box->border[BOTTOM].width) >
			visible_height) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_height;

	सचे (दक्षिणः) {
		सचे (box->scroll_y == रिक्त) {
			data = malloc(परिमाण(संरचना html_scrollbar_data));
			सचे (data == रिक्त) {
				पटिनिवत्त NSERROR_NOMEM;
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त res;
			}
		} अञ्ञथा  {
			scrollbar_set_extents(box->scroll_y,
					      visible_height,
					      visible_height,
					      full_height);
		}
	}
	सचे (bottom) {
		सचे (box->scroll_x == रिक्त) {
			data = malloc(परिमाण(संरचना html_scrollbar_data));
			सचे (data == रिक्त) {
				पटिनिवत्त NSERROR_OK;
			}
			data->c = c;
			data->box = box;
			res = scrollbar_create(true,
					       visible_width - (दक्षिणः ? SCROLLBAR_WIDTH : 0),
					       full_width,
					       visible_width,
					       data,
					       html_overflow_scroll_callback,
					       &box->scroll_x);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त res;
			}
		} अञ्ञथा {
			scrollbar_set_extents(box->scroll_x,
					visible_width -
					(दक्षिणः ? SCROLLBAR_WIDTH : 0),
					visible_width, full_width);
		}
	}

	सचे (दक्षिणः && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	पटिनिवत्त NSERROR_OK;
}


