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
	بنية html_scrollbar_داده *داده;

	إذا ((b->flags & STYLE_OWNED) && b->style != عدم) {
		css_computed_style_destroy(b->style);
		b->style = عدم;
	}

	إذا (b->styles != عدم) {
		css_select_results_destroy(b->styles);
		b->styles = عدم;
	}

	إذا (b->href != عدم)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	إذا (b->گره != عدم) {
		dom_گره_unref(b->گره);
	}

	إذا (b->scroll_x != عدم) {
		داده = scrollbar_get_داده(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		حرر(داده);
	}

	إذا (b->scroll_y != عدم) {
		داده = scrollbar_get_داده(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		حرر(داده);
	}

	أرجع 0;
}


/* Exported function documented in html/box.h */
بنية box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت حرف *هدف,
	   ثابت حرف *title,
	   lwc_string *id,
	   فراغ *context)
{
	بیعلامت صحيح i;
	بنية box *box;

	box = talloc(context, بنية box);
	إذا (!box) {
		أرجع 0;
	}

	talloc_مجموعه_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->عرض = UNKNOWN_عرض;
	box->ارتفاع = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	لكل (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].عرض = 0;
	box->scroll_x = box->scroll_y = عدم;
	box->min_عرض = 0;
	box->max_عرض = UNKNOWN_MAX_عرض;
	box->byte_جابه_جایی = 0;
	box->متن = عدم;
	box->درازی = 0;
	box->space = 0;
	box->href = (href == عدم) ? عدم : nsurl_ref(href);
	box->هدف = هدف;
	box->title = title;
	box->columns = 1;
	box->ردیفها = 1;
	box->شروع_ستونی = 0;
	box->بعدی = عدم;
	box->prev = عدم;
	box->children = عدم;
	box->آخرین = عدم;
	box->والد = عدم;
	box->inline_انتها = عدم;
	box->float_children = عدم;
	box->float_container = عدم;
	box->بعدی_float = عدم;
	box->cached_قراربده_زیر_level = 0;
	box->list_مقدار = 1;
	box->list_marker = عدم;
	box->ستون = عدم;
	box->gadget = عدم;
	box->usemap = عدم;
	box->id = id;
	box->background = عدم;
	box->object = عدم;
	box->object_params = عدم;
	box->iframe = عدم;
	box->گره = عدم;

	أرجع box;
}


/* Exported function documented in html/box.h */
فراغ box_جمع_کن_فرزند(بنية box *والد, بنية box *فرزند)
{
	أكد(والد);
	أكد(فرزند);

	إذا (والد->children != 0) {	/* has children already */
		والد->آخرین->بعدی = فرزند;
		فرزند->prev = والد->آخرین;
	} وإلا {			/* this is the first child */
		والد->children = فرزند;
		فرزند->prev = 0;
	}

	والد->آخرین = فرزند;
	فرزند->والد = والد;
}


/* Exported function documented in html/box.h */
فراغ box_درج_sibling(بنية box *box, بنية box *new_box)
{
	new_box->والد = box->والد;
	new_box->prev = box;
	new_box->بعدی = box->بعدی;
	box->بعدی = new_box;
	إذا (new_box->بعدی)
		new_box->بعدی->prev = new_box;
	وإلا إذا (new_box->والد)
		new_box->والد->آخرین = new_box;
}


/* Exported function documented in html/box.h */
فراغ box_unlink_and_free(بنية box *box)
{
	بنية box *والد = box->والد;
	بنية box *بعدی = box->بعدی;
	بنية box *prev = box->prev;

	إذا (والد) {
		إذا (والد->children == box)
			والد->children = بعدی;
		إذا (والد->آخرین == box)
			والد->آخرین = بعدی ? بعدی : prev;
	}

	إذا (prev)
		prev->بعدی = بعدی;
	إذا (بعدی)
		بعدی->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
فراغ box_free(بنية box *box)
{
	بنية box *فرزند, *بعدی;

	/* free children first */
	لكل (فرزند = box->children; فرزند; فرزند = بعدی) {
		بعدی = فرزند->بعدی;
		box_free(فرزند);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
فراغ box_free_box(بنية box *box)
{
	إذا (!(box->flags & CLONE)) {
		إذا (box->gadget)
			form_free_control(box->gadget);
		إذا (box->scroll_x != عدم)
			scrollbar_destroy(box->scroll_x);
		إذا (box->scroll_y != عدم)
			scrollbar_destroy(box->scroll_y);
		إذا (box->styles != عدم)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(بنية content *c,
		      بنية box *box,
		      bool bottom,
		      bool راست)
{
	بنية html_scrollbar_داده *داده;
	صحيح visible_عرض, visible_ارتفاع;
	صحيح پر_عرض, پر_ارتفاع;
	nserror res;

	إذا (!bottom && box->scroll_x != عدم) {
		داده = scrollbar_get_داده(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		حرر(داده);
		box->scroll_x = عدم;
	}

	إذا (!راست && box->scroll_y != عدم) {
		داده = scrollbar_get_داده(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		حرر(داده);
		box->scroll_y = عدم;
	}

	إذا (!bottom && !راست) {
		أرجع NSERROR_OK;
	}

	visible_عرض = box->عرض + box->padding[راست_2] + box->padding[چپ_2];
	visible_ارتفاع = box->ارتفاع + box->padding[بالاترین_2] + box->padding[BOTTOM];

	پر_عرض = ((box->descendant_x1 - box->border[راست_2].عرض) >
			visible_عرض) ?
			box->descendant_x1 + box->padding[راست_2] :
			visible_عرض;
	پر_ارتفاع = ((box->descendant_y1 - box->border[BOTTOM].عرض) >
			visible_ارتفاع) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_ارتفاع;

	إذا (راست) {
		إذا (box->scroll_y == عدم) {
			داده = حافظه_بده(حجم(بنية html_scrollbar_داده));
			إذا (داده == عدم) {
				أرجع NSERROR_NOMEM;
			}
			داده->c = c;
			داده->box = box;
			res = scrollbar_create(false,
					       visible_ارتفاع,
					       پر_ارتفاع,
					       visible_ارتفاع,
					       داده,
					       html_سرریز_scroll_callback,
					       &(box->scroll_y));
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا  {
			scrollbar_مجموعه_extents(box->scroll_y,
					      visible_ارتفاع,
					      visible_ارتفاع,
					      پر_ارتفاع);
		}
	}
	إذا (bottom) {
		إذا (box->scroll_x == عدم) {
			داده = حافظه_بده(حجم(بنية html_scrollbar_داده));
			إذا (داده == عدم) {
				أرجع NSERROR_OK;
			}
			داده->c = c;
			داده->box = box;
			res = scrollbar_create(true,
					       visible_عرض - (راست ? SCROLLBAR_عرض : 0),
					       پر_عرض,
					       visible_عرض,
					       داده,
					       html_سرریز_scroll_callback,
					       &box->scroll_x);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا {
			scrollbar_مجموعه_extents(box->scroll_x,
					visible_عرض -
					(راست ? SCROLLBAR_عرض : 0),
					visible_عرض, پر_عرض);
		}
	}

	إذا (راست && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	أرجع NSERROR_OK;
}


