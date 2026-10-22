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
	بنية html_scrollbar_انکڑے *انکڑے;

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

	إذا (b->گرہ != عدم) {
		dom_گرہ_unref(b->گرہ);
	}

	إذا (b->scroll_x != عدم) {
		انکڑے = scrollbar_get_انکڑے(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		حرر(انکڑے);
	}

	إذا (b->scroll_y != عدم) {
		انکڑے = scrollbar_get_انکڑے(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		حرر(انکڑے);
	}

	أرجع 0;
}


/* Exported function documented in html/box.h */
بنية box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت حرف *نشانہ,
	   ثابت حرف *title,
	   lwc_string *id,
	   فراغ *context)
{
	بنانشانی صحيح i;
	بنية box *box;

	box = talloc(context, بنية box);
	إذا (!box) {
		أرجع 0;
	}

	talloc_سیٹ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->چوڑائی = UNKNOWN_چوڑائی;
	box->اچائی = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	لكل (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].چوڑائی = 0;
	box->scroll_x = box->scroll_y = عدم;
	box->min_چوڑائی = 0;
	box->max_چوڑائی = UNKNOWN_MAX_چوڑائی;
	box->byte_وتھ = 0;
	box->متن = عدم;
	box->لمائی = 0;
	box->space = 0;
	box->href = (href == عدم) ? عدم : nsurl_ref(href);
	box->نشانہ = نشانہ;
	box->title = title;
	box->columns = 1;
	box->صفاں = 1;
	box->شروع_کالم = 0;
	box->اگلا = عدم;
	box->prev = عدم;
	box->children = عدم;
	box->آخری = عدم;
	box->ماپا = عدم;
	box->inline_سرا = عدم;
	box->float_children = عدم;
	box->float_container = عدم;
	box->اگلا_float = عدم;
	box->cached_رکھو_ہیٹھاں_level = 0;
	box->list_قدر = 1;
	box->list_marker = عدم;
	box->خانہ = عدم;
	box->gadget = عدم;
	box->usemap = عدم;
	box->id = id;
	box->background = عدم;
	box->object = عدم;
	box->object_params = عدم;
	box->iframe = عدم;
	box->گرہ = عدم;

	أرجع box;
}


/* Exported function documented in html/box.h */
فراغ box_جوڑو_بچہ(بنية box *ماپا, بنية box *بچہ)
{
	أكد(ماپا);
	أكد(بچہ);

	إذا (ماپا->children != 0) {	/* has children already */
		ماپا->آخری->اگلا = بچہ;
		بچہ->prev = ماپا->آخری;
	} وإلا {			/* this is the first child */
		ماپا->children = بچہ;
		بچہ->prev = 0;
	}

	ماپا->آخری = بچہ;
	بچہ->ماپا = ماپا;
}


/* Exported function documented in html/box.h */
فراغ box_داخل_کرو_sibling(بنية box *box, بنية box *new_box)
{
	new_box->ماپا = box->ماپا;
	new_box->prev = box;
	new_box->اگلا = box->اگلا;
	box->اگلا = new_box;
	إذا (new_box->اگلا)
		new_box->اگلا->prev = new_box;
	وإلا إذا (new_box->ماپا)
		new_box->ماپا->آخری = new_box;
}


/* Exported function documented in html/box.h */
فراغ box_unlink_and_free(بنية box *box)
{
	بنية box *ماپا = box->ماپا;
	بنية box *اگلا = box->اگلا;
	بنية box *prev = box->prev;

	إذا (ماپا) {
		إذا (ماپا->children == box)
			ماپا->children = اگلا;
		إذا (ماپا->آخری == box)
			ماپا->آخری = اگلا ? اگلا : prev;
	}

	إذا (prev)
		prev->اگلا = اگلا;
	إذا (اگلا)
		اگلا->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
فراغ box_free(بنية box *box)
{
	بنية box *بچہ, *اگلا;

	/* free children first */
	لكل (بچہ = box->children; بچہ; بچہ = اگلا) {
		اگلا = بچہ->اگلا;
		box_free(بچہ);
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
		      bool دایاں)
{
	بنية html_scrollbar_انکڑے *انکڑے;
	صحيح visible_چوڑائی, visible_اچائی;
	صحيح بھریا_چوڑائی, بھریا_اچائی;
	nserror res;

	إذا (!bottom && box->scroll_x != عدم) {
		انکڑے = scrollbar_get_انکڑے(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		حرر(انکڑے);
		box->scroll_x = عدم;
	}

	إذا (!دایاں && box->scroll_y != عدم) {
		انکڑے = scrollbar_get_انکڑے(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		حرر(انکڑے);
		box->scroll_y = عدم;
	}

	إذا (!bottom && !دایاں) {
		أرجع NSERROR_OK;
	}

	visible_چوڑائی = box->چوڑائی + box->padding[دایاں_2] + box->padding[بایاں_2];
	visible_اچائی = box->اچائی + box->padding[چوٹی_2] + box->padding[BOTTOM];

	بھریا_چوڑائی = ((box->descendant_x1 - box->border[دایاں_2].چوڑائی) >
			visible_چوڑائی) ?
			box->descendant_x1 + box->padding[دایاں_2] :
			visible_چوڑائی;
	بھریا_اچائی = ((box->descendant_y1 - box->border[BOTTOM].چوڑائی) >
			visible_اچائی) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_اچائی;

	إذا (دایاں) {
		إذا (box->scroll_y == عدم) {
			انکڑے = حافظہ_دیو(حجم(بنية html_scrollbar_انکڑے));
			إذا (انکڑے == عدم) {
				أرجع NSERROR_NOMEM;
			}
			انکڑے->c = c;
			انکڑے->box = box;
			res = scrollbar_create(false,
					       visible_اچائی,
					       بھریا_اچائی,
					       visible_اچائی,
					       انکڑے,
					       html_حد_توں_ودھ_scroll_callback,
					       &(box->scroll_y));
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا  {
			scrollbar_سیٹ_extents(box->scroll_y,
					      visible_اچائی,
					      visible_اچائی,
					      بھریا_اچائی);
		}
	}
	إذا (bottom) {
		إذا (box->scroll_x == عدم) {
			انکڑے = حافظہ_دیو(حجم(بنية html_scrollbar_انکڑے));
			إذا (انکڑے == عدم) {
				أرجع NSERROR_OK;
			}
			انکڑے->c = c;
			انکڑے->box = box;
			res = scrollbar_create(true,
					       visible_چوڑائی - (دایاں ? SCROLLBAR_چوڑائی : 0),
					       بھریا_چوڑائی,
					       visible_چوڑائی,
					       انکڑے,
					       html_حد_توں_ودھ_scroll_callback,
					       &box->scroll_x);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا {
			scrollbar_سیٹ_extents(box->scroll_x,
					visible_چوڑائی -
					(دایاں ? SCROLLBAR_چوڑائی : 0),
					visible_چوڑائی, بھریا_چوڑائی);
		}
	}

	إذا (دایاں && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	أرجع NSERROR_OK;
}


