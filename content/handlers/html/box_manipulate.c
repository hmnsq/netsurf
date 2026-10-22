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
	بنية html_scrollbar_معلومات *معلومات;

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

	إذا (b->غوټه != عدم) {
		dom_غوټه_unref(b->غوټه);
	}

	إذا (b->scroll_x != عدم) {
		معلومات = scrollbar_get_معلومات(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		حرر(معلومات);
	}

	إذا (b->scroll_y != عدم) {
		معلومات = scrollbar_get_معلومات(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		حرر(معلومات);
	}

	أرجع 0;
}


/* Exported function documented in html/box.h */
بنية box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت حرف *موخه,
	   ثابت حرف *title,
	   lwc_string *id,
	   فراغ *context)
{
	بینښه صحيح i;
	بنية box *box;

	box = talloc(context, بنية box);
	إذا (!box) {
		أرجع 0;
	}

	talloc_سېټ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->پلنوالی = UNKNOWN_پلنوالی;
	box->لوړوالی = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	لكل (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].پلنوالی = 0;
	box->scroll_x = box->scroll_y = عدم;
	box->min_پلنوالی = 0;
	box->max_پلنوالی = UNKNOWN_MAX_پلنوالی;
	box->byte_دننه_واټن = 0;
	box->متن = عدم;
	box->اوږدتیا = 0;
	box->space = 0;
	box->href = (href == عدم) ? عدم : nsurl_ref(href);
	box->موخه = موخه;
	box->title = title;
	box->columns = 1;
	box->لیکې = 1;
	box->پیل_ستنه = 0;
	box->بل = عدم;
	box->prev = عدم;
	box->children = عدم;
	box->وروستی = عدم;
	box->پلار = عدم;
	box->inline_پای_ټکی = عدم;
	box->float_children = عدم;
	box->float_container = عدم;
	box->بل_float = عدم;
	box->cached_ځای_پرځای_کړه_لاندې_level = 0;
	box->list_ارزښت = 1;
	box->list_marker = عدم;
	box->ستون = عدم;
	box->gadget = عدم;
	box->usemap = عدم;
	box->id = id;
	box->background = عدم;
	box->object = عدم;
	box->object_params = عدم;
	box->iframe = عدم;
	box->غوټه = عدم;

	أرجع box;
}


/* Exported function documented in html/box.h */
فراغ box_جمع_کړه_ماشوم(بنية box *پلار, بنية box *ماشوم)
{
	أكد(پلار);
	أكد(ماشوم);

	إذا (پلار->children != 0) {	/* has children already */
		پلار->وروستی->بل = ماشوم;
		ماشوم->prev = پلار->وروستی;
	} وإلا {			/* this is the first child */
		پلار->children = ماشوم;
		ماشوم->prev = 0;
	}

	پلار->وروستی = ماشوم;
	ماشوم->پلار = پلار;
}


/* Exported function documented in html/box.h */
فراغ box_ورننباسه_sibling(بنية box *box, بنية box *new_box)
{
	new_box->پلار = box->پلار;
	new_box->prev = box;
	new_box->بل = box->بل;
	box->بل = new_box;
	إذا (new_box->بل)
		new_box->بل->prev = new_box;
	وإلا إذا (new_box->پلار)
		new_box->پلار->وروستی = new_box;
}


/* Exported function documented in html/box.h */
فراغ box_unlink_and_free(بنية box *box)
{
	بنية box *پلار = box->پلار;
	بنية box *بل = box->بل;
	بنية box *prev = box->prev;

	إذا (پلار) {
		إذا (پلار->children == box)
			پلار->children = بل;
		إذا (پلار->وروستی == box)
			پلار->وروستی = بل ? بل : prev;
	}

	إذا (prev)
		prev->بل = بل;
	إذا (بل)
		بل->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
فراغ box_free(بنية box *box)
{
	بنية box *ماشوم, *بل;

	/* free children first */
	لكل (ماشوم = box->children; ماشوم; ماشوم = بل) {
		بل = ماشوم->بل;
		box_free(ماشوم);
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
		      bool ښی)
{
	بنية html_scrollbar_معلومات *معلومات;
	صحيح visible_پلنوالی, visible_لوړوالی;
	صحيح ډک_پلنوالی, ډک_لوړوالی;
	nserror res;

	إذا (!bottom && box->scroll_x != عدم) {
		معلومات = scrollbar_get_معلومات(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		حرر(معلومات);
		box->scroll_x = عدم;
	}

	إذا (!ښی && box->scroll_y != عدم) {
		معلومات = scrollbar_get_معلومات(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		حرر(معلومات);
		box->scroll_y = عدم;
	}

	إذا (!bottom && !ښی) {
		أرجع NSERROR_OK;
	}

	visible_پلنوالی = box->پلنوالی + box->padding[ښی_2] + box->padding[کیڼ_2];
	visible_لوړوالی = box->لوړوالی + box->padding[سر_2] + box->padding[BOTTOM];

	ډک_پلنوالی = ((box->descendant_x1 - box->border[ښی_2].پلنوالی) >
			visible_پلنوالی) ?
			box->descendant_x1 + box->padding[ښی_2] :
			visible_پلنوالی;
	ډک_لوړوالی = ((box->descendant_y1 - box->border[BOTTOM].پلنوالی) >
			visible_لوړوالی) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_لوړوالی;

	إذا (ښی) {
		إذا (box->scroll_y == عدم) {
			معلومات = حافظه_ورکړه(حجم(بنية html_scrollbar_معلومات));
			إذا (معلومات == عدم) {
				أرجع NSERROR_NOMEM;
			}
			معلومات->c = c;
			معلومات->box = box;
			res = scrollbar_create(false,
					       visible_لوړوالی,
					       ډک_لوړوالی,
					       visible_لوړوالی,
					       معلومات,
					       html_ډکېدنه_scroll_callback,
					       &(box->scroll_y));
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا  {
			scrollbar_سېټ_extents(box->scroll_y,
					      visible_لوړوالی,
					      visible_لوړوالی,
					      ډک_لوړوالی);
		}
	}
	إذا (bottom) {
		إذا (box->scroll_x == عدم) {
			معلومات = حافظه_ورکړه(حجم(بنية html_scrollbar_معلومات));
			إذا (معلومات == عدم) {
				أرجع NSERROR_OK;
			}
			معلومات->c = c;
			معلومات->box = box;
			res = scrollbar_create(true,
					       visible_پلنوالی - (ښی ? SCROLLBAR_پلنوالی : 0),
					       ډک_پلنوالی,
					       visible_پلنوالی,
					       معلومات,
					       html_ډکېدنه_scroll_callback,
					       &box->scroll_x);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا {
			scrollbar_سېټ_extents(box->scroll_x,
					visible_پلنوالی -
					(ښی ? SCROLLBAR_پلنوالی : 0),
					visible_پلنوالی, ډک_پلنوالی);
		}
	}

	إذا (ښی && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	أرجع NSERROR_OK;
}


