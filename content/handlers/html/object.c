/*
 * Copyright 2013 Vincent Sanders <vince@netsurf-browser.org>
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
 * Processing for html content object operations.
 */

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"

/* break reference loop */
ಸ್ಥಿರ ಶೂನ್ಯ html_object_refresh(ಶೂನ್ಯ *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ರಚನೆ content_html_object *html_get_objects(hlcache_handle *h, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(n != ಖಾಲಿ);

	*n = c->ಅಂಕೆ_objects;

	ಹಿಂತಿರುಗಿ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_object_failed(ರಚನೆ box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ಹಿಂತಿರುಗಿ;
}

/**
 * Update a box whose content has completed rendering.
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_object_ಮುಗಿದ(ರಚನೆ box *box,
		 hlcache_handle *object,
		 bool background)
{
	ರಚನೆ box *b;

	ಆದರೆ (background) {
		box->background = object;
		ಹಿಂತಿರುಗಿ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	ಆಯ್ಕೆ (box->type) {
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		box->type = BOX_ಖಂಡ;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* TODO: Any other box types need mapping? */
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		ಗಾಗಿ (b = box; b; b = b->ಪೋಷಕ)
			b->max_ಅಗಲ = UNKNOWN_MAX_ಅಗಲ;

		/* delete any clones of this box */
		ಆಗ (box->ಮುಂದಿನ && (box->ಮುಂದಿನ->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->ಮುಂದಿನ = box->ಮುಂದಿನ->ಮುಂದಿನ;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ಸ್ಥಿರ nserror
html_object_nobox_callback(hlcache_handle *object,
			   ಸ್ಥಿರಾಂಕ hlcache_event *event,
			   ಶೂನ್ಯ *pw)
{
	ರಚನೆ content_html_object *chobject = pw;

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(object);

		chobject->content = ಖಾಲಿ;
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ಸ್ಥಿರ nserror
html_object_callback(hlcache_handle *object,
		     ಸ್ಥಿರಾಂಕ hlcache_event *event,
		     ಶೂನ್ಯ *pw)
{
	ರಚನೆ content_html_object *o = pw;
	html_content *c = (html_content *) o->ಪೋಷಕ;
	ಪೂರ್ಣಾಂಕ x, y;
	ರಚನೆ box *box;

	box = o->box;

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_LOADING:
		ಆದರೆ (c->ತಳ.ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_LOADING && c->bw != ಖಾಲಿ)
			content_open(object,
					c->bw, &c->ತಳ,
					box->object_params);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_READY:
		ಆದರೆ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ ?
							box->ಅಗಲ : 0,
					box->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ ?
							box->ಎತ್ತರ : 0);

			/* Adjust parent content for new object size */
			html_object_ಮುಗಿದ(box, object, o->background);
			ಆದರೆ (c->ತಳ.ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
					c->ತಳ.ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)
				content__reformat(&c->ತಳ, false,
						c->ತಳ.ಲಭ್ಯ_ಅಗಲ,
						c->ತಳ.ಲಭ್ಯ_ಎತ್ತರ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ಮುಗಿದ:
		c->ತಳ.ಸಕ್ರಿಯ--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

		html_object_ಮುಗಿದ(box, object, o->background);

		ಆದರೆ (c->ತಳ.ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_LOADING &&
				box->flags & REPLACE_DIM) {
			ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;

			ಆದರೆ (c->had_initial_layout == false) {
				ನಿಲ್ಲಿಸು;
			}

			ಆದರೆ (!box_visible(box))
				ನಿಲ್ಲಿಸು;

			box_coords(box, &x, &y);

			ದತ್ತಾಂಶ.redraw.x = x + box->padding[ಎಡ_2];
			ದತ್ತಾಂಶ.redraw.y = y + box->padding[ತುದಿ_2];
			ದತ್ತಾಂಶ.redraw.ಅಗಲ = box->ಅಗಲ;
			ದತ್ತಾಂಶ.redraw.ಎತ್ತರ = box->ಎತ್ತರ;

			content_ಪ್ರಸಾರ(&c->ತಳ, CONTENT_ಸಂದೇಶ_REDRAW, &ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(object);

		o->content = ಖಾಲಿ;

		c->ತಳ.ಸಕ್ರಿಯ--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

		html_object_failed(box, c, o->background);

		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_REDRAW:
		ಆದರೆ (c->ತಳ.ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_LOADING) {
			ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ದತ್ತಾಂಶ = event->ದತ್ತಾಂಶ;

			ಆದರೆ (c->had_initial_layout == false) {
				ನಿಲ್ಲಿಸು;
			}

			ಆದರೆ (!box_visible(box))
				ನಿಲ್ಲಿಸು;

			box_coords(box, &x, &y);

			ಆದರೆ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				ಪೂರ್ಣಾಂಕ ಅಗಲ = box->padding[ಎಡ_2] + box->ಅಗಲ +
						box->padding[ಬಲ_2];
				ಪೂರ್ಣಾಂಕ ಎತ್ತರ = box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
						box->padding[BOTTOM];
				ಪೂರ್ಣಾಂಕ t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_ಅಗಲ(box->background);
				ಆದರೆ (hunit == CSS_UNIT_PCT) {
					l = (ಅಗಲ - w) * hpos / INTTOFIX(100);
				} ಇಲ್ಲದಿದ್ದರೆ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_ಎತ್ತರ(box->background);
				ಆದರೆ (vunit == CSS_UNIT_PCT) {
					t = (ಎತ್ತರ - h) * vpos / INTTOFIX(100);
				} ಇಲ್ಲದಿದ್ದರೆ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				ಆಯ್ಕೆ (css_computed_background_repeat(
						box->style)) {
				ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT:
					ದತ್ತಾಂಶ.redraw.x = 0;
					ದತ್ತಾಂಶ.redraw.y = 0;
					ದತ್ತಾಂಶ.redraw.ಅಗಲ = box->ಅಗಲ;
					ದತ್ತಾಂಶ.redraw.ಎತ್ತರ = box->ಎತ್ತರ;
					ನಿಲ್ಲಿಸು;

				ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_X:
					ದತ್ತಾಂಶ.redraw.x = 0;
					ದತ್ತಾಂಶ.redraw.y += t;
					ದತ್ತಾಂಶ.redraw.ಅಗಲ = box->ಅಗಲ;
					ನಿಲ್ಲಿಸು;

				ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					ದತ್ತಾಂಶ.redraw.x += l;
					ದತ್ತಾಂಶ.redraw.y = 0;
					ದತ್ತಾಂಶ.redraw.ಎತ್ತರ = box->ಎತ್ತರ;
					ನಿಲ್ಲಿಸು;

				ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					ದತ್ತಾಂಶ.redraw.x += l;
					ದತ್ತಾಂಶ.redraw.y += t;
					ನಿಲ್ಲಿಸು;

				ಪೂರ್ವನಿಯೋಜಿತ:
					ನಿಲ್ಲಿಸು;
				}

				/* Add offset to box */
				ದತ್ತಾಂಶ.redraw.x += x;
				ದತ್ತಾಂಶ.redraw.y += y;

			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Non-background case */
				ಪೂರ್ಣಾಂಕ w = content_get_ಅಗಲ(object);
				ಪೂರ್ಣಾಂಕ h = content_get_ಎತ್ತರ(object);

				ಆದರೆ (w != 0 && box->ಅಗಲ != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					ದತ್ತಾಂಶ.redraw.x = ದತ್ತಾಂಶ.redraw.x *
							box->ಅಗಲ / w;
					ದತ್ತಾಂಶ.redraw.ಅಗಲ =
							ದತ್ತಾಂಶ.redraw.ಅಗಲ *
							box->ಅಗಲ / w;
				}

				ಆದರೆ (h != 0 && box->ಎತ್ತರ != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					ದತ್ತಾಂಶ.redraw.y = ದತ್ತಾಂಶ.redraw.y *
							box->ಎತ್ತರ / h;
					ದತ್ತಾಂಶ.redraw.ಎತ್ತರ =
							ದತ್ತಾಂಶ.redraw.ಎತ್ತರ *
							box->ಎತ್ತರ / h;
				}

				ದತ್ತಾಂಶ.redraw.x += x + box->padding[ಎಡ_2];
				ದತ್ತಾಂಶ.redraw.y += y + box->padding[ತುದಿ_2];
			}

			content_ಪ್ರಸಾರ(&c->ತಳ, CONTENT_ಸಂದೇಶ_REDRAW, &ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_REFRESH:
		ಆದರೆ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->ವೇಳಾಪಟ್ಟಿ(event->ದತ್ತಾಂಶ.delay * 1000,
					html_object_refresh, o);
		}

		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_LINK:
		/* Don't care about favicons that aren't on top level content */
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->ದತ್ತಾಂಶ.jsthread) = ಖಾಲಿ;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_GETDIMS:
		*(event->ದತ್ತಾಂಶ.getdims.viewport_ಅಗಲ) =
				content_get_ಅಗಲ_2(&c->ತಳ);
		*(event->ದತ್ತಾಂಶ.getdims.viewport_ಎತ್ತರ) =
				content_get_ಎತ್ತರ_2(&c->ತಳ);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_SCROLL:
		ಆದರೆ (box->scroll_x != ಖಾಲಿ)
			scrollbar_ಗಣ(box->scroll_x, event->ದತ್ತಾಂಶ.scroll.x0,
					false);
		ಆದರೆ (box->scroll_y != ಖಾಲಿ)
			scrollbar_ಗಣ(box->scroll_y, event->ದತ್ತಾಂಶ.scroll.y0,
					false);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_DRAGSAVE:
	{
		ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
		ಆದರೆ (event->ದತ್ತಾಂಶ.dragsave.content == ಖಾಲಿ)
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content = object;
		ಇಲ್ಲದಿದ್ದರೆ
			ಸಂದೇಶ_ದತ್ತಾಂಶ.dragsave.content =
					event->ದತ್ತಾಂಶ.dragsave.content;

		content_ಪ್ರಸಾರ(&c->ತಳ, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_SAVELINK:
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ಸೂಚಕ:
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_SELECTMENU:
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_ಪ್ರಸಾರ(&c->ತಳ, event->type, &event->ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_CARET:
	{
		ಒಕ್ಕೂಟ html_focus_owner focus_owner;
		focus_owner.content = box;

		ಆಯ್ಕೆ (event->ದತ್ತಾಂಶ.caret.type) {
		ಸ್ಥಿತಿ CONTENT_CARET_REMOVE:
		ಸ್ಥಿತಿ CONTENT_CARET_HIDE:
			html_ಗಣ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ಖಾಲಿ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CONTENT_CARET_ಗಣ_POS:
			html_ಗಣ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->ದತ್ತಾಂಶ.caret.pos.x,
					event->ದತ್ತಾಂಶ.caret.pos.y,
					event->ದತ್ತಾಂಶ.caret.pos.ಎತ್ತರ,
					event->ದತ್ತಾಂಶ.caret.pos.clip);
			ನಿಲ್ಲಿಸು;
		}
	}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		ಒಕ್ಕೂಟ html_drag_owner drag_owner;
		drag_owner.content = box;

		ಆಯ್ಕೆ (event->ದತ್ತಾಂಶ.drag.type) {
		ಸ್ಥಿತಿ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ನಿಲ್ಲಿಸು;
		}
		html_ಗಣ_drag_type(c, drag_type, drag_owner,
				event->ದತ್ತಾಂಶ.drag.rect);
	}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_SELECTION:
	{
		html_selection_type sel_type;
		ಒಕ್ಕೂಟ html_selection_owner sel_owner;

		ಆದರೆ (event->ದತ್ತಾಂಶ.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ಇಲ್ಲದಿದ್ದರೆ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_ಗಣ_selection(c, sel_type, sel_owner,
				event->ದತ್ತಾಂಶ.selection.read_only);
	}
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (c->ತಳ.ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY &&
	    c->ತಳ.ಸಕ್ರಿಯ == 0 &&
	    (event->type == CONTENT_ಸಂದೇಶ_LOADING ||
	     event->type == CONTENT_ಸಂದೇಶ_ಮುಗಿದ ||
	     event->type == CONTENT_ಸಂದೇಶ_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->ತಳ, false, c->ತಳ.ಲಭ್ಯ_ಅಗಲ,
				c->ತಳ.ಲಭ್ಯ_ಎತ್ತರ);
		content_ಗಣ_ಮುಗಿದ(&c->ತಳ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_ಸಂದೇಶ_ಮುಗಿದ &&
		   box != ಖಾಲಿ &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->ತಳ.ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
		    c->ತಳ.ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		ಆದರೆ (ms_now > c->ತಳ.reformat_ಸಮಯ) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->ತಳ,
					  false,
					  c->ತಳ.ಲಭ್ಯ_ಅಗಲ,
					  c->ತಳ.ಲಭ್ಯ_ಎತ್ತರ);
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool html_replace_object(ರಚನೆ content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_ಮಗು_context ಮಗು;
	html_content *ಪುಟ;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(object != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(object->box != ಖಾಲಿ);

	c = (html_content *) object->ಪೋಷಕ;

	ಮಗು.charset = c->encoding;
	ಮಗು.quirks = c->ತಳ.quirks;

	ಆದರೆ (object->content != ಖಾಲಿ) {
		/* remove existing object */
		ಆದರೆ (content_get_ಸ್ಥಿತಿ(object->content) != CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
			c->ತಳ.ಸಕ್ರಿಯ--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->ತಳ.ಸಕ್ರಿಯ);
		}

		hlcache_handle_release(object->content);
		object->content = ಖಾಲಿ;

		object->box->object = ಖಾಲಿ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->ತಳ), ಖಾಲಿ,
			html_object_callback, object, &ಮಗು,
			object->permitted_types,
			&object->content);

	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (ಪುಟ = c; ಪುಟ != ಖಾಲಿ; ಪುಟ = ಪುಟ->ಪುಟ) {
		ಪುಟ->ತಳ.ಸಕ್ರಿಯ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

		ಪುಟ->ತಳ.ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_READY;
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * schedule callback for object refresh
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_object_refresh(ಶೂನ್ಯ *p)
{
	ರಚನೆ content_html_object *object = p;
	nsurl *refresh_url;

	ಪ್ರತಿಜ್ಞಾ(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ಆದರೆ (refresh_url == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	content_invalidate_reuse_ದತ್ತಾಂಶ(object->content);

	ಆದರೆ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ರಚನೆ browser_ಕಿಟಕಿ *bw)
{
	ರಚನೆ content_html_object *object, *ಮುಂದಿನ;

	ಗಾಗಿ (object = html->object_list; object != ಖಾಲಿ; object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಆದರೆ (object->content == ಖಾಲಿ || object->box == ಖಾಲಿ)
			ಮುಂದುವರಿ;

		ಆದರೆ (content_get_type(object->content) == CONTENT_NONE)
			ಮುಂದುವರಿ;

		content_open(object->content,
			     bw,
			     &html->ತಳ,
			     object->box->object_params);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ರಚನೆ content_html_object *object;

	ಗಾಗಿ (object = htmlc->object_list;
	     object != ಖಾಲಿ;
	     object = object->ಮುಂದಿನ) {
		ಆದರೆ (object->content == ಖಾಲಿ)
			ಮುಂದುವರಿ;

		ಆಯ್ಕೆ (content_get_ಸ್ಥಿತಿ(object->content)) {
		ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ:
			/* already loaded: do nothing */
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ಖಾಲಿ;
			ಆದರೆ (object->box != ಖಾಲಿ) {
				htmlc->ತಳ.ಸಕ್ರಿಯ--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->ತಳ.ಸಕ್ರಿಯ);
			}
			ನಿಲ್ಲಿಸು;

		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ರಚನೆ content_html_object *object, *ಮುಂದಿನ;

	ಗಾಗಿ (object = html->object_list; object != ಖಾಲಿ; object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಆದರೆ (object->content == ಖಾಲಿ || object->box == ಖಾಲಿ)
			ಮುಂದುವರಿ;

		ಆದರೆ (content_get_type(object->content) == CONTENT_NONE)
			ಮುಂದುವರಿ;

		ಆದರೆ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ಆಗ (html->object_list != ಖಾಲಿ) {
		ರಚನೆ content_html_object *victim = html->object_list;

		ಆದರೆ (victim->content != ಖಾಲಿ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ಆದರೆ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(victim);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ರಚನೆ box *box,
		  content_type permitted_types,
		  bool background)
{
	ರಚನೆ content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_ಮಗು_context ಮಗು;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ಆದರೆ (c->aborted)
		ಹಿಂತಿರುಗಿ true;

	ಮಗು.charset = c->encoding;
	ಮಗು.quirks = c->ತಳ.quirks;

	object = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content_html_object));
	ಆದರೆ (object == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (box == ಖಾಲಿ) {
		object_callback = html_object_nobox_callback;
	} ಇಲ್ಲದಿದ್ದರೆ {
		object_callback = html_object_callback;
	}

	object->ಪೋಷಕ = (ರಚನೆ content *) c;
	object->ಮುಂದಿನ = ಖಾಲಿ;
	object->content = ಖಾಲಿ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->ತಳ),
					ಖಾಲಿ,
					object_callback,
					object,
					&ಮಗು,
					object->permitted_types,
					&object->content);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(object);
		ಹಿಂತಿರುಗಿ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->ಮುಂದಿನ = c->object_list;
	c->object_list = object;

	c->ಅಂಕೆ_objects++;
	ಆದರೆ (box != ಖಾಲಿ) {
		c->ತಳ.ಸಕ್ರಿಯ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);
	}

	ಹಿಂತಿರುಗಿ true;
}
