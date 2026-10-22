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


#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"


/**
 * Destructor for box nodes which own styles
 *
 * \param b The box being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ box_talloc_destructor(ರಚನೆ box *b)
{
	ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;

	ಆದರೆ ((b->flags & STYLE_OWNED) && b->style != ಖಾಲಿ) {
		css_computed_style_destroy(b->style);
		b->style = ಖಾಲಿ;
	}

	ಆದರೆ (b->styles != ಖಾಲಿ) {
		css_select_results_destroy(b->styles);
		b->styles = ಖಾಲಿ;
	}

	ಆದರೆ (b->href != ಖಾಲಿ)
		nsurl_unref(b->href);

	lwc_string_unref(b->id);

	ಆದರೆ (b->ಗಂಟು != ಖಾಲಿ) {
		dom_ಗಂಟು_unref(b->ಗಂಟು);
	}

	ಆದರೆ (b->scroll_x != ಖಾಲಿ) {
		ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(b->scroll_x);
		scrollbar_destroy(b->scroll_x);
		ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
	}

	ಆದರೆ (b->scroll_y != ಖಾಲಿ) {
		ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(b->scroll_y);
		scrollbar_destroy(b->scroll_y);
		ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ 0;
}


/* Exported function documented in html/box.h */
ರಚನೆ box *
box_create(css_select_results *styles,
	   css_computed_style *style,
	   bool style_owned,
	   nsurl *href,
	   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ,
	   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title,
	   lwc_string *id,
	   ಶೂನ್ಯ *context)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ box *box;

	box = talloc(context, ರಚನೆ box);
	ಆದರೆ (!box) {
		ಹಿಂತಿರುಗಿ 0;
	}

	talloc_ಗಣ_destructor(box, box_talloc_destructor);

	box->type = BOX_INLINE;
	box->flags = 0;
	box->flags = style_owned ? (box->flags | STYLE_OWNED) : box->flags;
	box->styles = styles;
	box->style = style;
	box->x = box->y = 0;
	box->ಅಗಲ = UNKNOWN_ಅಗಲ;
	box->ಎತ್ತರ = 0;
	box->descendant_x0 = box->descendant_y0 = 0;
	box->descendant_x1 = box->descendant_y1 = 0;
	ಗಾಗಿ (i = 0; i != 4; i++)
		box->margin[i] = box->padding[i] = box->border[i].ಅಗಲ = 0;
	box->scroll_x = box->scroll_y = ಖಾಲಿ;
	box->min_ಅಗಲ = 0;
	box->max_ಅಗಲ = UNKNOWN_MAX_ಅಗಲ;
	box->byte_ಸ್ಥಾನಾಂತರ = 0;
	box->ಪಠ್ಯ = ಖಾಲಿ;
	box->ಉದ್ದಳತೆ = 0;
	box->space = 0;
	box->href = (href == ಖಾಲಿ) ? ಖಾಲಿ : nsurl_ref(href);
	box->ಗುರಿ = ಗುರಿ;
	box->title = title;
	box->columns = 1;
	box->ಅಡ್ಡಸಾಲುಗಳು = 1;
	box->ಆರಂಭ_ಸ್ತಂಭ = 0;
	box->ಮುಂದಿನ = ಖಾಲಿ;
	box->prev = ಖಾಲಿ;
	box->children = ಖಾಲಿ;
	box->ಕೊನೆಯ = ಖಾಲಿ;
	box->ಪೋಷಕ = ಖಾಲಿ;
	box->inline_ಕೊನೆ = ಖಾಲಿ;
	box->float_children = ಖಾಲಿ;
	box->float_container = ಖಾಲಿ;
	box->ಮುಂದಿನ_float = ಖಾಲಿ;
	box->cached_ಇರಿಸು_ಕೆಳಗೆ_level = 0;
	box->list_ಮೌಲ್ಯ = 1;
	box->list_marker = ಖಾಲಿ;
	box->ಲಂಬಸಾಲು = ಖಾಲಿ;
	box->gadget = ಖಾಲಿ;
	box->usemap = ಖಾಲಿ;
	box->id = id;
	box->background = ಖಾಲಿ;
	box->object = ಖಾಲಿ;
	box->object_params = ಖಾಲಿ;
	box->iframe = ಖಾಲಿ;
	box->ಗಂಟು = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ box;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_ಕೂಡಿಸು_ಮಗು(ರಚನೆ box *ಪೋಷಕ, ರಚನೆ box *ಮಗು)
{
	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ);
	ಪ್ರತಿಜ್ಞಾ(ಮಗು);

	ಆದರೆ (ಪೋಷಕ->children != 0) {	/* has children already */
		ಪೋಷಕ->ಕೊನೆಯ->ಮುಂದಿನ = ಮಗು;
		ಮಗು->prev = ಪೋಷಕ->ಕೊನೆಯ;
	} ಇಲ್ಲದಿದ್ದರೆ {			/* this is the first child */
		ಪೋಷಕ->children = ಮಗು;
		ಮಗು->prev = 0;
	}

	ಪೋಷಕ->ಕೊನೆಯ = ಮಗು;
	ಮಗು->ಪೋಷಕ = ಪೋಷಕ;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_ಸೇರಿಸು_sibling(ರಚನೆ box *box, ರಚನೆ box *new_box)
{
	new_box->ಪೋಷಕ = box->ಪೋಷಕ;
	new_box->prev = box;
	new_box->ಮುಂದಿನ = box->ಮುಂದಿನ;
	box->ಮುಂದಿನ = new_box;
	ಆದರೆ (new_box->ಮುಂದಿನ)
		new_box->ಮುಂದಿನ->prev = new_box;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (new_box->ಪೋಷಕ)
		new_box->ಪೋಷಕ->ಕೊನೆಯ = new_box;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_unlink_and_free(ರಚನೆ box *box)
{
	ರಚನೆ box *ಪೋಷಕ = box->ಪೋಷಕ;
	ರಚನೆ box *ಮುಂದಿನ = box->ಮುಂದಿನ;
	ರಚನೆ box *prev = box->prev;

	ಆದರೆ (ಪೋಷಕ) {
		ಆದರೆ (ಪೋಷಕ->children == box)
			ಪೋಷಕ->children = ಮುಂದಿನ;
		ಆದರೆ (ಪೋಷಕ->ಕೊನೆಯ == box)
			ಪೋಷಕ->ಕೊನೆಯ = ಮುಂದಿನ ? ಮುಂದಿನ : prev;
	}

	ಆದರೆ (prev)
		prev->ಮುಂದಿನ = ಮುಂದಿನ;
	ಆದರೆ (ಮುಂದಿನ)
		ಮುಂದಿನ->prev = prev;

	box_free(box);
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_free(ರಚನೆ box *box)
{
	ರಚನೆ box *ಮಗು, *ಮುಂದಿನ;

	/* free children first */
	ಗಾಗಿ (ಮಗು = box->children; ಮಗು; ಮಗು = ಮುಂದಿನ) {
		ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;
		box_free(ಮಗು);
	}

	/* last this box */
	box_free_box(box);
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_free_box(ರಚನೆ box *box)
{
	ಆದರೆ (!(box->flags & CLONE)) {
		ಆದರೆ (box->gadget)
			form_free_control(box->gadget);
		ಆದರೆ (box->scroll_x != ಖಾಲಿ)
			scrollbar_destroy(box->scroll_x);
		ಆದರೆ (box->scroll_y != ಖಾಲಿ)
			scrollbar_destroy(box->scroll_y);
		ಆದರೆ (box->styles != ಖಾಲಿ)
			css_select_results_destroy(box->styles);
	}

	talloc_free(box);
}


/* exported interface documented in html/box.h */
nserror
box_handle_scrollbars(ರಚನೆ content *c,
		      ರಚನೆ box *box,
		      bool bottom,
		      bool ಬಲ)
{
	ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ visible_ಅಗಲ, visible_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ ತುಂಬಿದ_ಅಗಲ, ತುಂಬಿದ_ಎತ್ತರ;
	nserror res;

	ಆದರೆ (!bottom && box->scroll_x != ಖಾಲಿ) {
		ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(box->scroll_x);
		scrollbar_destroy(box->scroll_x);
		ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
		box->scroll_x = ಖಾಲಿ;
	}

	ಆದರೆ (!ಬಲ && box->scroll_y != ಖಾಲಿ) {
		ದತ್ತಾಂಶ = scrollbar_get_ದತ್ತಾಂಶ(box->scroll_y);
		scrollbar_destroy(box->scroll_y);
		ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
		box->scroll_y = ಖಾಲಿ;
	}

	ಆದರೆ (!bottom && !ಬಲ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	visible_ಅಗಲ = box->ಅಗಲ + box->padding[ಬಲ_2] + box->padding[ಎಡ_2];
	visible_ಎತ್ತರ = box->ಎತ್ತರ + box->padding[ತುದಿ_2] + box->padding[BOTTOM];

	ತುಂಬಿದ_ಅಗಲ = ((box->descendant_x1 - box->border[ಬಲ_2].ಅಗಲ) >
			visible_ಅಗಲ) ?
			box->descendant_x1 + box->padding[ಬಲ_2] :
			visible_ಅಗಲ;
	ತುಂಬಿದ_ಎತ್ತರ = ((box->descendant_y1 - box->border[BOTTOM].ಅಗಲ) >
			visible_ಎತ್ತರ) ?
			box->descendant_y1 + box->padding[BOTTOM] :
			visible_ಎತ್ತರ;

	ಆದರೆ (ಬಲ) {
		ಆದರೆ (box->scroll_y == ಖಾಲಿ) {
			ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ));
			ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ) {
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
			ದತ್ತಾಂಶ->c = c;
			ದತ್ತಾಂಶ->box = box;
			res = scrollbar_create(false,
					       visible_ಎತ್ತರ,
					       ತುಂಬಿದ_ಎತ್ತರ,
					       visible_ಎತ್ತರ,
					       ದತ್ತಾಂಶ,
					       html_ಅತಿಪ್ರವಾಹ_scroll_callback,
					       &(box->scroll_y));
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}
		} ಇಲ್ಲದಿದ್ದರೆ  {
			scrollbar_ಗಣ_extents(box->scroll_y,
					      visible_ಎತ್ತರ,
					      visible_ಎತ್ತರ,
					      ತುಂಬಿದ_ಎತ್ತರ);
		}
	}
	ಆದರೆ (bottom) {
		ಆದರೆ (box->scroll_x == ಖಾಲಿ) {
			ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ html_scrollbar_ದತ್ತಾಂಶ));
			ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ) {
				ಹಿಂತಿರುಗಿ NSERROR_OK;
			}
			ದತ್ತಾಂಶ->c = c;
			ದತ್ತಾಂಶ->box = box;
			res = scrollbar_create(true,
					       visible_ಅಗಲ - (ಬಲ ? SCROLLBAR_ಅಗಲ : 0),
					       ತುಂಬಿದ_ಅಗಲ,
					       visible_ಅಗಲ,
					       ದತ್ತಾಂಶ,
					       html_ಅತಿಪ್ರವಾಹ_scroll_callback,
					       &box->scroll_x);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			scrollbar_ಗಣ_extents(box->scroll_x,
					visible_ಅಗಲ -
					(ಬಲ ? SCROLLBAR_ಅಗಲ : 0),
					visible_ಅಗಲ, ತುಂಬಿದ_ಅಗಲ);
		}
	}

	ಆದರೆ (ಬಲ && bottom) {
		scrollbar_make_pair(box->scroll_x, box->scroll_y);
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


