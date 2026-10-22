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


#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/interaction.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ਸਥਿਰ ਪੂਰਨ box_talloc_destructor(ਬਣਤਰ box *b)
{
	ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ *ਅੰਕੜੇ;

	ਜੇ ((b->flags & STYLE_OWNED) && b->style != ਨਿਰਾ) {
		css_computed_style_destroy(b->style);
		b->style = ਨਿਰਾ;
	}

	ਜੇ (b->styles != ਨਿਰਾ) {
		css_select_results_destroy(b->styles);
		b->styles = ਨਿਰਾ;
	}

	ਜੇ (b->href != ਨਿਰਾ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ਜੇ (b->ਗੰਢ != ਨਿਰਾ) {
		dom_ਗੰਢ_unref(b->ਗੰਢ);
	}

	ਜੇ (b->scroll_x != ਨਿਰਾ) {
		ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		ਮੁਕਤ(ਅੰਕੜੇ);
	}

	ਜੇ (b->scroll_y != ਨਿਰਾ) {
		ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		ਮੁਕਤ(ਅੰਕੜੇ);
	}

	ਵਾਪਸ 0;
}


/* Exported function documented in html/box.h */
ਬਣਤਰ box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ,
	   ਅਚਲ ਅੱਖਰ *title,
	   lwc_string *id,
	   ਖਾਲੀ *context)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ box *box;

	box = talloc(context, ਬਣਤਰ box);
	ਜੇ (!box) {
		ਵਾਪਸ 0;
	}

	talloc_ਸੈੱਟ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->ਚੌੜਾਈ = UNKNOWN_ਚੌੜਾਈ;
	box->ਉਚਾਈ = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	ਲਈ (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].ਚੌੜਾਈ = 0;
	box->scroll_x = box->scroll_y = ਨਿਰਾ;
	box->min_ਚੌੜਾਈ = 0;
	box->max_ਚੌੜਾਈ = UNKNOWN_MAX_ਚੌੜਾਈ;
	box->byte_ਵਿੱਥ = 0;
	box->ਪਾਠ = ਨਿਰਾ;
	box->ਲੰਮਾਈ = 0;
	box->space = 0;
	box->href = (href == ਨਿਰਾ) ? ਨਿਰਾ : nsurl_ref(href);
	box->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
	box->title = title;
	box->columns = 1;
	box->ਪੰਕਤੀਆਂ = 1;
	box->ਸ਼ੁਰੂ_ਕਾਲਮ = 0;
	box->ਅਗਲਾ = ਨਿਰਾ;
	box->prev = ਨਿਰਾ;
	box->children = ਨਿਰਾ;
	box->ਆਖਰੀ = ਨਿਰਾ;
	box->ਮਾਪਾ = ਨਿਰਾ;
	box->inline_ਸਿਰਾ = ਨਿਰਾ;
	box->float_children = ਨਿਰਾ;
	box->float_container = ਨਿਰਾ;
	box->ਅਗਲਾ_float = ਨਿਰਾ;
	box->cached_ਰੱਖੋ_ਹੇਠਾਂ_level = 0;
	box->list_ਮੁੱਲ = 1;
	box->list_marker = ਨਿਰਾ;
	box->ਖਾਨਾ = ਨਿਰਾ;
	box->gadget = ਨਿਰਾ;
	box->usemap = ਨਿਰਾ;
	box->id = id;
	box->background = ਨਿਰਾ;
	box->object = ਨਿਰਾ;
	box->object_params = ਨਿਰਾ;
	box->iframe = ਨਿਰਾ;
	box->ਗੰਢ = ਨਿਰਾ;

	ਵਾਪਸ box;
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_ਜੋੜੋ_ਬੱਚਾ(ਬਣਤਰ box *ਮਾਪਾ, ਬਣਤਰ box *ਬੱਚਾ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਬੱਚਾ);

	ਜੇ (ਮਾਪਾ->children != 0) {	/* has children already */
		ਮਾਪਾ->ਆਖਰੀ->ਅਗਲਾ = ਬੱਚਾ;
		ਬੱਚਾ->prev = ਮਾਪਾ->ਆਖਰੀ;
	} ਨਹੀਂਤਾਂ {			/* this is the first child */
		ਮਾਪਾ->children = ਬੱਚਾ;
		ਬੱਚਾ->prev = 0;
	}

	ਮਾਪਾ->ਆਖਰੀ = ਬੱਚਾ;
	ਬੱਚਾ->ਮਾਪਾ = ਮਾਪਾ;
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_ਪਾਓ_sibling(ਬਣਤਰ box *box, ਬਣਤਰ box *new_box)
{
	new_box->ਮਾਪਾ = box->ਮਾਪਾ;
	new_box->prev = box;
	new_box->ਅਗਲਾ = box->ਅਗਲਾ;
	box->ਅਗਲਾ = new_box;
	ਜੇ (new_box->ਅਗਲਾ)
		new_box->ਅਗਲਾ->prev = new_box;
	ਨਹੀਂਤਾਂ ਜੇ (new_box->ਮਾਪਾ)
		new_box->ਮਾਪਾ->ਆਖਰੀ = new_box;
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_unlink_and_free(ਬਣਤਰ box *box)
{
	ਬਣਤਰ box *ਮਾਪਾ = box->ਮਾਪਾ;
	ਬਣਤਰ box *ਅਗਲਾ = box->ਅਗਲਾ;
	ਬਣਤਰ box *prev = box->prev;

	ਜੇ (ਮਾਪਾ) {
		ਜੇ (ਮਾਪਾ->children == box)
			ਮਾਪਾ->children = ਅਗਲਾ;
		ਜੇ (ਮਾਪਾ->ਆਖਰੀ == box)
			ਮਾਪਾ->ਆਖਰੀ = ਅਗਲਾ ? ਅਗਲਾ : prev;
	}

	ਜੇ (prev)
		prev->ਅਗਲਾ = ਅਗਲਾ;
	ਜੇ (ਅਗਲਾ)
		ਅਗਲਾ->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_free(ਬਣਤਰ box *box)
{
	ਬਣਤਰ box *ਬੱਚਾ, *ਅਗਲਾ;

	/* free children first */
	ਲਈ (ਬੱਚਾ = box->children; ਬੱਚਾ; ਬੱਚਾ = ਅਗਲਾ) {
		ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;
		box_free(ਬੱਚਾ);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_free_box(ਬਣਤਰ box *box)
{
	ਜੇ (!(box->flags & CLONE)) {
		ਜੇ (box->gadget)
			form_free_control(box->gadget);
		ਜੇ (box->scroll_x != ਨਿਰਾ)
			scrollbar_destroy(box->scroll_x);
		ਜੇ (box->scroll_y != ਨਿਰਾ)
			scrollbar_destroy(box->scroll_y);
		ਜੇ (box->styles != ਨਿਰਾ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ਬਣਤਰ content *c,
		      ਬਣਤਰ box *box,
		      bool bottom,
		      bool ਸੱਜਾ)
{
	ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਪੂਰਨ visible_ਚੌੜਾਈ, visible_ਉਚਾਈ;
	ਪੂਰਨ ਭਰਿਆ_ਚੌੜਾਈ, ਭਰਿਆ_ਉਚਾਈ;
	nserror res;

	ਜੇ (!bottom && box->scroll_x != ਨਿਰਾ) {
		ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		ਮੁਕਤ(ਅੰਕੜੇ);
		box->scroll_x = ਨਿਰਾ;
	}

	ਜੇ (!ਸੱਜਾ && box->scroll_y != ਨਿਰਾ) {
		ਅੰਕੜੇ = scrollbar_get_ਅੰਕੜੇ(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		ਮੁਕਤ(ਅੰਕੜੇ);
		box->scroll_y = ਨਿਰਾ;
	}

	ਜੇ (!bottom && !ਸੱਜਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	visible_ਚੌੜਾਈ = box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] + box->padding[ਖੱਬਾ_2];
	visible_ਉਚਾਈ = box->ਉਚਾਈ + box->padding[ਸਿਖਰ_2] + box->padding[BOTTOM];

	ਭਰਿਆ_ਚੌੜਾਈ = ((box->descendant_x1 - box->border[ਸੱਜਾ_2].ਚੌੜਾਈ) >
			visible_ਚੌੜਾਈ) ?
			box->descendant_x1 + box->padding[ਸੱਜਾ_2] :
			visible_ਚੌੜਾਈ;
	ਭਰਿਆ_ਉਚਾਈ = ((box->descendant_y1 - box->border[BOTTOM].ਚੌੜਾਈ) >
			visible_ਉਚਾਈ) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_ਉਚਾਈ;

	ਜੇ (ਸੱਜਾ) {
		ਜੇ (box->scroll_y == ਨਿਰਾ) {
			ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ));
			ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ) {
				ਵਾਪਸ NSERROR_NOMEM;
			}
			ਅੰਕੜੇ->c = c;
			ਅੰਕੜੇ->box = box;
			res = scrollbar_create(false,
					       visible_ਉਚਾਈ,
					       ਭਰਿਆ_ਉਚਾਈ,
					       visible_ਉਚਾਈ,
					       ਅੰਕੜੇ,
					       html_ਉਛਾਲ_scroll_callback,
					       &(box->scroll_y));
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}
		} ਨਹੀਂਤਾਂ  {
			scrollbar_ਸੈੱਟ_extents(box->scroll_y,
					      visible_ਉਚਾਈ,
					      visible_ਉਚਾਈ,
					      ਭਰਿਆ_ਉਚਾਈ);
		}
	}
	ਜੇ (bottom) {
		ਜੇ (box->scroll_x == ਨਿਰਾ) {
			ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ html_scrollbar_ਅੰਕੜੇ));
			ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ) {
				ਵਾਪਸ NSERROR_OK;
			}
			ਅੰਕੜੇ->c = c;
			ਅੰਕੜੇ->box = box;
			res = scrollbar_create(true,
					       visible_ਚੌੜਾਈ - (ਸੱਜਾ ? SCROLLBAR_ਚੌੜਾਈ : 0),
					       ਭਰਿਆ_ਚੌੜਾਈ,
					       visible_ਚੌੜਾਈ,
					       ਅੰਕੜੇ,
					       html_ਉਛਾਲ_scroll_callback,
					       &box->scroll_x);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}
		} ਨਹੀਂਤਾਂ {
			scrollbar_ਸੈੱਟ_extents(box->scroll_x,
					visible_ਚੌੜਾਈ -
					(ਸੱਜਾ ? SCROLLBAR_ਚੌੜਾਈ : 0),
					visible_ਚੌੜਾਈ, ਭਰਿਆ_ਚੌੜਾਈ);
		}
	}

	ਜੇ (ਸੱਜਾ && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	ਵਾਪਸ NSERROR_OK;
}


