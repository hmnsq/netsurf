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

	إذا (b->ڳنڍ != عدم) {
		dom_ڳنڍ_unref(b->ڳنڍ);
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
	   ثابت حرف *هدف,
	   ثابت حرف *title,
	   lwc_string *id,
	   فراغ *context)
{
	بنانشان صحيح i;
	بنية box *box;

	box = talloc(context, بنية box);
	إذا (!box) {
		أرجع 0;
	}

	talloc_سيٽ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->ويڪر = UNKNOWN_ويڪر;
	box->اوچائي = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	لكل (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].ويڪر = 0;
	box->scroll_x = box->scroll_y = عدم;
	box->min_ويڪر = 0;
	box->max_ويڪر = UNKNOWN_MAX_ويڪر;
	box->byte_وٿي = 0;
	box->متن = عدم;
	box->ڊيگهائي = 0;
	box->space = 0;
	box->href = (href == عدم) ? عدم : nsurl_ref(href);
	box->هدف = هدف;
	box->title = title;
	box->columns = 1;
	box->صفون = 1;
	box->شروعات_ڪالم = 0;
	box->ايندڙ = عدم;
	box->prev = عدم;
	box->children = عدم;
	box->آخري = عدم;
	box->والد = عدم;
	box->inline_پڇاڙي = عدم;
	box->float_children = عدم;
	box->float_container = عدم;
	box->ايندڙ_float = عدم;
	box->cached_رکو_هيٺ_level = 0;
	box->list_قدر = 1;
	box->list_marker = عدم;
	box->کانو = عدم;
	box->gadget = عدم;
	box->usemap = عدم;
	box->id = id;
	box->background = عدم;
	box->object = عدم;
	box->object_params = عدم;
	box->iframe = عدم;
	box->ڳنڍ = عدم;

	أرجع box;
}


/* Exported function documented in html/box.h */
فراغ box_جوڙيو_ٻار(بنية box *والد, بنية box *ٻار)
{
	أكد(والد);
	أكد(ٻار);

	إذا (والد->children != 0) {	/* has children already */
		والد->آخري->ايندڙ = ٻار;
		ٻار->prev = والد->آخري;
	} وإلا {			/* this is the first child */
		والد->children = ٻار;
		ٻار->prev = 0;
	}

	والد->آخري = ٻار;
	ٻار->والد = والد;
}


/* Exported function documented in html/box.h */
فراغ box_وجهو_sibling(بنية box *box, بنية box *new_box)
{
	new_box->والد = box->والد;
	new_box->prev = box;
	new_box->ايندڙ = box->ايندڙ;
	box->ايندڙ = new_box;
	إذا (new_box->ايندڙ)
		new_box->ايندڙ->prev = new_box;
	وإلا إذا (new_box->والد)
		new_box->والد->آخري = new_box;
}


/* Exported function documented in html/box.h */
فراغ box_unlink_and_free(بنية box *box)
{
	بنية box *والد = box->والد;
	بنية box *ايندڙ = box->ايندڙ;
	بنية box *prev = box->prev;

	إذا (والد) {
		إذا (والد->children == box)
			والد->children = ايندڙ;
		إذا (والد->آخري == box)
			والد->آخري = ايندڙ ? ايندڙ : prev;
	}

	إذا (prev)
		prev->ايندڙ = ايندڙ;
	إذا (ايندڙ)
		ايندڙ->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
فراغ box_free(بنية box *box)
{
	بنية box *ٻار, *ايندڙ;

	/* free children first */
	لكل (ٻار = box->children; ٻار; ٻار = ايندڙ) {
		ايندڙ = ٻار->ايندڙ;
		box_free(ٻار);
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
		      bool ساڄو)
{
	بنية html_scrollbar_معلومات *معلومات;
	صحيح visible_ويڪر, visible_اوچائي;
	صحيح ڀريل_ويڪر, ڀريل_اوچائي;
	nserror res;

	إذا (!bottom && box->scroll_x != عدم) {
		معلومات = scrollbar_get_معلومات(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		حرر(معلومات);
		box->scroll_x = عدم;
	}

	إذا (!ساڄو && box->scroll_y != عدم) {
		معلومات = scrollbar_get_معلومات(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		حرر(معلومات);
		box->scroll_y = عدم;
	}

	إذا (!bottom && !ساڄو) {
		أرجع NSERROR_OK;
	}

	visible_ويڪر = box->ويڪر + box->padding[ساڄو_2] + box->padding[کاٻو_2];
	visible_اوچائي = box->اوچائي + box->padding[چوٽي_2] + box->padding[BOTTOM];

	ڀريل_ويڪر = ((box->descendant_x1 - box->border[ساڄو_2].ويڪر) >
			visible_ويڪر) ?
			box->descendant_x1 + box->padding[ساڄو_2] :
			visible_ويڪر;
	ڀريل_اوچائي = ((box->descendant_y1 - box->border[BOTTOM].ويڪر) >
			visible_اوچائي) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_اوچائي;

	إذا (ساڄو) {
		إذا (box->scroll_y == عدم) {
			معلومات = يادگيري_ڏيو(حجم(بنية html_scrollbar_معلومات));
			إذا (معلومات == عدم) {
				أرجع NSERROR_NOMEM;
			}
			معلومات->c = c;
			معلومات->box = box;
			res = scrollbar_create(false,
					       visible_اوچائي,
					       ڀريل_اوچائي,
					       visible_اوچائي,
					       معلومات,
					       html_حد_کان_مٿي_scroll_callback,
					       &(box->scroll_y));
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا  {
			scrollbar_سيٽ_extents(box->scroll_y,
					      visible_اوچائي,
					      visible_اوچائي,
					      ڀريل_اوچائي);
		}
	}
	إذا (bottom) {
		إذا (box->scroll_x == عدم) {
			معلومات = يادگيري_ڏيو(حجم(بنية html_scrollbar_معلومات));
			إذا (معلومات == عدم) {
				أرجع NSERROR_OK;
			}
			معلومات->c = c;
			معلومات->box = box;
			res = scrollbar_create(true,
					       visible_ويڪر - (ساڄو ? SCROLLBAR_ويڪر : 0),
					       ڀريل_ويڪر,
					       visible_ويڪر,
					       معلومات,
					       html_حد_کان_مٿي_scroll_callback,
					       &box->scroll_x);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		} وإلا {
			scrollbar_سيٽ_extents(box->scroll_x,
					visible_ويڪر -
					(ساڄو ? SCROLLBAR_ويڪر : 0),
					visible_ويڪر, ڀريل_ويڪر);
		}
	}

	إذا (ساڄو && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	أرجع NSERROR_OK;
}


