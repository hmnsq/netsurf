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


#شامل "utils/errors.h"
#شامل "utils/talloc.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/types.h"
#شامل "netsurf/mouse.h"
#شامل "desktop/scrollbar.h"

#شامل "html/private.h"
#شامل "html/form_internal.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساکن صحیح box_talloc_destructor(ساخت box *b)
{
	ساخت html_scrollbar_ڈیٹا *ڈیٹا;

	اگر ((b->flags & STYLE_OWNED) && b->style != تہی) {
		css_computed_style_destroy(b->style);
		b->style = تہی;
	}

	اگر (b->styles != تہی) {
		css_select_results_destroy(b->styles);
		b->styles = تہی;
	}

	اگر (b->href != تہی)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	اگر (b->گرہ != تہی) {
		dom_گرہ_unref(b->گرہ);
	}

	اگر (b->scroll_x != تہی) {
		ڈیٹا = scrollbar_get_ڈیٹا(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		آزاد(ڈیٹا);
	}

	اگر (b->scroll_y != تہی) {
		ڈیٹا = scrollbar_get_ڈیٹا(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		آزاد(ڈیٹا);
	}

	واپس 0;
}


/* Exported function documented in html/box.h */
ساخت box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ثابت حرف *ہدف,
	   ثابت حرف *title,
	   lwc_string *id,
	   خالی *context)
{
	غیرعلامتی صحیح i;
	ساخت box *box;

	box = talloc(context, ساخت box);
	اگر (!box) {
		واپس 0;
	}

	talloc_سیٹ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->چوڑائی = UNKNOWN_چوڑائی;
	box->اونچائی = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	برائے (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].چوڑائی = 0;
	box->scroll_x = box->scroll_y = تہی;
	box->min_چوڑائی = 0;
	box->max_چوڑائی = UNKNOWN_MAX_چوڑائی;
	box->byte_انحراف = 0;
	box->متن = تہی;
	box->لمبائی = 0;
	box->space = 0;
	box->href = (href == تہی) ? تہی : nsurl_ref(href);
	box->ہدف = ہدف;
	box->title = title;
	box->columns = 1;
	box->صفیں = 1;
	box->آغاز_عمود = 0;
	box->اگلا = تہی;
	box->پچھلا = تہی;
	box->children = تہی;
	box->آخری = تہی;
	box->والد = تہی;
	box->inline_انتہا = تہی;
	box->float_children = تہی;
	box->float_container = تہی;
	box->اگلا_float = تہی;
	box->cached_رکھو_نیچے_level = 0;
	box->list_قدر = 1;
	box->list_marker = تہی;
	box->ستون = تہی;
	box->gadget = تہی;
	box->usemap = تہی;
	box->id = id;
	box->background = تہی;
	box->object = تہی;
	box->object_params = تہی;
	box->iframe = تہی;
	box->گرہ = تہی;

	واپس box;
}


/* Exported function documented in html/box.h */
خالی box_جوڑو_بچہ(ساخت box *والد, ساخت box *بچہ)
{
	أكد(والد);
	أكد(بچہ);

	اگر (والد->children != 0) {	/* has children already */
		والد->آخری->اگلا = بچہ;
		بچہ->پچھلا = والد->آخری;
	} ورنہ {			/* this is the first child */
		والد->children = بچہ;
		بچہ->پچھلا = 0;
	}

	والد->آخری = بچہ;
	بچہ->والد = والد;
}


/* Exported function documented in html/box.h */
خالی box_داخل_کرو_sibling(ساخت box *box, ساخت box *new_box)
{
	new_box->والد = box->والد;
	new_box->پچھلا = box;
	new_box->اگلا = box->اگلا;
	box->اگلا = new_box;
	اگر (new_box->اگلا)
		new_box->اگلا->پچھلا = new_box;
	ورنہ اگر (new_box->والد)
		new_box->والد->آخری = new_box;
}


/* Exported function documented in html/box.h */
خالی box_unlink_and_free(ساخت box *box)
{
	ساخت box *والد = box->والد;
	ساخت box *اگلا = box->اگلا;
	ساخت box *پچھلا = box->پچھلا;

	اگر (والد) {
		اگر (والد->children == box)
			والد->children = اگلا;
		اگر (والد->آخری == box)
			والد->آخری = اگلا ? اگلا : پچھلا;
	}

	اگر (پچھلا)
		پچھلا->اگلا = اگلا;
	اگر (اگلا)
		اگلا->پچھلا = پچھلا;

	box_free(box);
}


/* Exported function documented in html/box.h */
خالی box_free(ساخت box *box)
{
	ساخت box *بچہ, *اگلا;

	/* free children first */
	برائے (بچہ = box->children; بچہ; بچہ = اگلا) {
		اگلا = بچہ->اگلا;
		box_free(بچہ);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
خالی box_free_box(ساخت box *box)
{
	اگر (!(box->flags & CLONE)) {
		اگر (box->gadget)
			form_free_control(box->gadget);
		اگر (box->scroll_x != تہی)
			scrollbar_destroy(box->scroll_x);
		اگر (box->scroll_y != تہی)
			scrollbar_destroy(box->scroll_y);
		اگر (box->styles != تہی)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ساخت content *c,
		      ساخت box *box,
		      bool bottom,
		      bool دایاں)
{
	ساخت html_scrollbar_ڈیٹا *ڈیٹا;
	صحیح visible_چوڑائی, visible_اونچائی;
	صحیح بھرا_چوڑائی, بھرا_اونچائی;
	nserror res;

	اگر (!bottom && box->scroll_x != تہی) {
		ڈیٹا = scrollbar_get_ڈیٹا(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		آزاد(ڈیٹا);
		box->scroll_x = تہی;
	}

	اگر (!دایاں && box->scroll_y != تہی) {
		ڈیٹا = scrollbar_get_ڈیٹا(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		آزاد(ڈیٹا);
		box->scroll_y = تہی;
	}

	اگر (!bottom && !دایاں) {
		واپس NSERROR_OK;
	}

	visible_چوڑائی = box->چوڑائی + box->padding[دایاں_2] + box->padding[بایاں_2];
	visible_اونچائی = box->اونچائی + box->padding[چوٹی_2] + box->padding[BOTTOM];

	بھرا_چوڑائی = ((box->descendant_x1 - box->border[دایاں_2].چوڑائی) >
			visible_چوڑائی) ?
			box->descendant_x1 + box->padding[دایاں_2] :
			visible_چوڑائی;
	بھرا_اونچائی = ((box->descendant_y1 - box->border[BOTTOM].چوڑائی) >
			visible_اونچائی) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_اونچائی;

	اگر (دایاں) {
		اگر (box->scroll_y == تہی) {
			ڈیٹا = حافظہ_دو(حجم(ساخت html_scrollbar_ڈیٹا));
			اگر (ڈیٹا == تہی) {
				واپس NSERROR_NOMEM;
			}
			ڈیٹا->c = c;
			ڈیٹا->box = box;
			res = scrollbar_create(false,
					       visible_اونچائی,
					       بھرا_اونچائی,
					       visible_اونچائی,
					       ڈیٹا,
					       html_حد_سے_تجاوز_scroll_callback,
					       &(box->scroll_y));
			اگر (res != NSERROR_OK) {
				واپس res;
			}
		} ورنہ  {
			scrollbar_سیٹ_extents(box->scroll_y,
					      visible_اونچائی,
					      visible_اونچائی,
					      بھرا_اونچائی);
		}
	}
	اگر (bottom) {
		اگر (box->scroll_x == تہی) {
			ڈیٹا = حافظہ_دو(حجم(ساخت html_scrollbar_ڈیٹا));
			اگر (ڈیٹا == تہی) {
				واپس NSERROR_OK;
			}
			ڈیٹا->c = c;
			ڈیٹا->box = box;
			res = scrollbar_create(true,
					       visible_چوڑائی - (دایاں ? SCROLLBAR_چوڑائی : 0),
					       بھرا_چوڑائی,
					       visible_چوڑائی,
					       ڈیٹا,
					       html_حد_سے_تجاوز_scroll_callback,
					       &box->scroll_x);
			اگر (res != NSERROR_OK) {
				واپس res;
			}
		} ورنہ {
			scrollbar_سیٹ_extents(box->scroll_x,
					visible_چوڑائی -
					(دایاں ? SCROLLBAR_چوڑائی : 0),
					visible_چوڑائی, بھرا_چوڑائی);
		}
	}

	اگر (دایاں && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	واپس NSERROR_OK;
}


