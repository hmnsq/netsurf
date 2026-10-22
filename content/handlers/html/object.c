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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/interaction.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/object.h"

/* break reference loop */
ਸਥਿਰ ਖਾਲੀ html_object_refresh(ਖਾਲੀ *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ਬਣਤਰ content_html_object *html_get_objects(hlcache_handle *h, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(n != ਨਿਰਾ);

	*n = c->ਅੰਕ_objects;

	ਵਾਪਸ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ਸਥਿਰ ਖਾਲੀ
html_object_failed(ਬਣਤਰ box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ਵਾਪਸ;
}

/**
 * Update a box whose content has completed rendering.
 */

ਸਥਿਰ ਖਾਲੀ
html_object_ਪੂਰਾ(ਬਣਤਰ box *box,
		 hlcache_handle *object,
		 bool background)
{
	ਬਣਤਰ box *b;

	ਜੇ (background) {
		box->background = object;
		ਵਾਪਸ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	ਚੋਣ (box->type) {
	ਹਾਲਤ BOX_ਸਾਰਣੀ:
		box->type = BOX_ਖੰਡ;
		ਰੋਕੋ;
	ਮੂਲ:
		/* TODO: Any other box types need mapping? */
		ਰੋਕੋ;
	}

	ਜੇ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		ਲਈ (b = box; b; b = b->ਮਾਪਾ)
			b->max_ਚੌੜਾਈ = UNKNOWN_MAX_ਚੌੜਾਈ;

		/* delete any clones of this box */
		ਜਦੋਂ (box->ਅਗਲਾ && (box->ਅਗਲਾ->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->ਅਗਲਾ = box->ਅਗਲਾ->ਅਗਲਾ;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ਸਥਿਰ nserror
html_object_nobox_callback(hlcache_handle *object,
			   ਅਚਲ hlcache_event *event,
			   ਖਾਲੀ *pw)
{
	ਬਣਤਰ content_html_object *chobject = pw;

	ਚੋਣ (event->type) {
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		hlcache_handle_release(object);

		chobject->content = ਨਿਰਾ;
		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ਸਥਿਰ nserror
html_object_callback(hlcache_handle *object,
		     ਅਚਲ hlcache_event *event,
		     ਖਾਲੀ *pw)
{
	ਬਣਤਰ content_html_object *o = pw;
	html_content *c = (html_content *) o->ਮਾਪਾ;
	ਪੂਰਨ x, y;
	ਬਣਤਰ box *box;

	box = o->box;

	ਚੋਣ (event->type) {
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_LOADING:
		ਜੇ (c->ਅਧਾਰ.ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_LOADING && c->bw != ਨਿਰਾ)
			content_open(object,
					c->bw, &c->ਅਧਾਰ,
					box->object_params);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_READY:
		ਜੇ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ ?
							box->ਚੌੜਾਈ : 0,
					box->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ ?
							box->ਉਚਾਈ : 0);

			/* Adjust parent content for new object size */
			html_object_ਪੂਰਾ(box, object, o->background);
			ਜੇ (c->ਅਧਾਰ.ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
					c->ਅਧਾਰ.ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)
				content__reformat(&c->ਅਧਾਰ, false,
						c->ਅਧਾਰ.ਉਪਲਬਧ_ਚੌੜਾਈ,
						c->ਅਧਾਰ.ਉਪਲਬਧ_ਉਚਾਈ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		c->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

		html_object_ਪੂਰਾ(box, object, o->background);

		ਜੇ (c->ਅਧਾਰ.ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_LOADING &&
				box->flags & REPLACE_DIM) {
			ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ;

			ਜੇ (c->had_initial_layout == false) {
				ਰੋਕੋ;
			}

			ਜੇ (!box_visible(box))
				ਰੋਕੋ;

			box_coords(box, &x, &y);

			ਅੰਕੜੇ.redraw.x = x + box->padding[ਖੱਬਾ_2];
			ਅੰਕੜੇ.redraw.y = y + box->padding[ਸਿਖਰ_2];
			ਅੰਕੜੇ.redraw.ਚੌੜਾਈ = box->ਚੌੜਾਈ;
			ਅੰਕੜੇ.redraw.ਉਚਾਈ = box->ਉਚਾਈ;

			content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_REDRAW, &ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		hlcache_handle_release(object);

		o->content = ਨਿਰਾ;

		c->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

		html_object_failed(box, c, o->background);

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_REDRAW:
		ਜੇ (c->ਅਧਾਰ.ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_LOADING) {
			ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ = event->ਅੰਕੜੇ;

			ਜੇ (c->had_initial_layout == false) {
				ਰੋਕੋ;
			}

			ਜੇ (!box_visible(box))
				ਰੋਕੋ;

			box_coords(box, &x, &y);

			ਜੇ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				ਪੂਰਨ ਚੌੜਾਈ = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ +
						box->padding[ਸੱਜਾ_2];
				ਪੂਰਨ ਉਚਾਈ = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
						box->padding[BOTTOM];
				ਪੂਰਨ t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_ਚੌੜਾਈ(box->background);
				ਜੇ (hunit == CSS_UNIT_PCT) {
					l = (ਚੌੜਾਈ - w) * hpos / INTTOFIX(100);
				} ਨਹੀਂਤਾਂ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_ਉਚਾਈ(box->background);
				ਜੇ (vunit == CSS_UNIT_PCT) {
					t = (ਉਚਾਈ - h) * vpos / INTTOFIX(100);
				} ਨਹੀਂਤਾਂ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				ਚੋਣ (css_computed_background_repeat(
						box->style)) {
				ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT:
					ਅੰਕੜੇ.redraw.x = 0;
					ਅੰਕੜੇ.redraw.y = 0;
					ਅੰਕੜੇ.redraw.ਚੌੜਾਈ = box->ਚੌੜਾਈ;
					ਅੰਕੜੇ.redraw.ਉਚਾਈ = box->ਉਚਾਈ;
					ਰੋਕੋ;

				ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_X:
					ਅੰਕੜੇ.redraw.x = 0;
					ਅੰਕੜੇ.redraw.y += t;
					ਅੰਕੜੇ.redraw.ਚੌੜਾਈ = box->ਚੌੜਾਈ;
					ਰੋਕੋ;

				ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					ਅੰਕੜੇ.redraw.x += l;
					ਅੰਕੜੇ.redraw.y = 0;
					ਅੰਕੜੇ.redraw.ਉਚਾਈ = box->ਉਚਾਈ;
					ਰੋਕੋ;

				ਹਾਲਤ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					ਅੰਕੜੇ.redraw.x += l;
					ਅੰਕੜੇ.redraw.y += t;
					ਰੋਕੋ;

				ਮੂਲ:
					ਰੋਕੋ;
				}

				/* Add offset to box */
				ਅੰਕੜੇ.redraw.x += x;
				ਅੰਕੜੇ.redraw.y += y;

			} ਨਹੀਂਤਾਂ {
				/* Non-background case */
				ਪੂਰਨ w = content_get_ਚੌੜਾਈ(object);
				ਪੂਰਨ h = content_get_ਉਚਾਈ(object);

				ਜੇ (w != 0 && box->ਚੌੜਾਈ != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					ਅੰਕੜੇ.redraw.x = ਅੰਕੜੇ.redraw.x *
							box->ਚੌੜਾਈ / w;
					ਅੰਕੜੇ.redraw.ਚੌੜਾਈ =
							ਅੰਕੜੇ.redraw.ਚੌੜਾਈ *
							box->ਚੌੜਾਈ / w;
				}

				ਜੇ (h != 0 && box->ਉਚਾਈ != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					ਅੰਕੜੇ.redraw.y = ਅੰਕੜੇ.redraw.y *
							box->ਉਚਾਈ / h;
					ਅੰਕੜੇ.redraw.ਉਚਾਈ =
							ਅੰਕੜੇ.redraw.ਉਚਾਈ *
							box->ਉਚਾਈ / h;
				}

				ਅੰਕੜੇ.redraw.x += x + box->padding[ਖੱਬਾ_2];
				ਅੰਕੜੇ.redraw.y += y + box->padding[ਸਿਖਰ_2];
			}

			content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_REDRAW, &ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_REFRESH:
		ਜੇ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(event->ਅੰਕੜੇ.delay * 1000,
					html_object_refresh, o);
		}

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_LINK:
		/* Don't care about favicons that aren't on top level content */
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->ਅੰਕੜੇ.jsthread) = ਨਿਰਾ;
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_GETDIMS:
		*(event->ਅੰਕੜੇ.getdims.viewport_ਚੌੜਾਈ) =
				content_get_ਚੌੜਾਈ_2(&c->ਅਧਾਰ);
		*(event->ਅੰਕੜੇ.getdims.viewport_ਉਚਾਈ) =
				content_get_ਉਚਾਈ_2(&c->ਅਧਾਰ);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_SCROLL:
		ਜੇ (box->scroll_x != ਨਿਰਾ)
			scrollbar_ਸੈੱਟ(box->scroll_x, event->ਅੰਕੜੇ.scroll.x0,
					false);
		ਜੇ (box->scroll_y != ਨਿਰਾ)
			scrollbar_ਸੈੱਟ(box->scroll_y, event->ਅੰਕੜੇ.scroll.y0,
					false);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_DRAGSAVE:
	{
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
		ਜੇ (event->ਅੰਕੜੇ.dragsave.content == ਨਿਰਾ)
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content = object;
		ਨਹੀਂਤਾਂ
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.dragsave.content =
					event->ਅੰਕੜੇ.dragsave.content;

		content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_DRAGSAVE, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_SAVELINK:
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ:
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_SELECTMENU:
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, event->type, &event->ਅੰਕੜੇ);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_CARET:
	{
		ਸੰਘ html_focus_owner focus_owner;
		focus_owner.content = box;

		ਚੋਣ (event->ਅੰਕੜੇ.caret.type) {
		ਹਾਲਤ CONTENT_CARET_REMOVE:
		ਹਾਲਤ CONTENT_CARET_HIDE:
			html_ਸੈੱਟ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ਨਿਰਾ);
			ਰੋਕੋ;
		ਹਾਲਤ CONTENT_CARET_ਸੈੱਟ_POS:
			html_ਸੈੱਟ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->ਅੰਕੜੇ.caret.pos.x,
					event->ਅੰਕੜੇ.caret.pos.y,
					event->ਅੰਕੜੇ.caret.pos.ਉਚਾਈ,
					event->ਅੰਕੜੇ.caret.pos.clip);
			ਰੋਕੋ;
		}
	}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		ਸੰਘ html_drag_owner drag_owner;
		drag_owner.content = box;

		ਚੋਣ (event->ਅੰਕੜੇ.drag.type) {
		ਹਾਲਤ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ਰੋਕੋ;
		ਹਾਲਤ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ਰੋਕੋ;
		ਹਾਲਤ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ਰੋਕੋ;
		}
		html_ਸੈੱਟ_drag_type(c, drag_type, drag_owner,
				event->ਅੰਕੜੇ.drag.rect);
	}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_SELECTION:
	{
		html_selection_type sel_type;
		ਸੰਘ html_selection_owner sel_owner;

		ਜੇ (event->ਅੰਕੜੇ.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ਨਹੀਂਤਾਂ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_ਸੈੱਟ_selection(c, sel_type, sel_owner,
				event->ਅੰਕੜੇ.selection.read_only);
	}
		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਜੇ (c->ਅਧਾਰ.ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY &&
	    c->ਅਧਾਰ.ਸਰਗਰਮ == 0 &&
	    (event->type == CONTENT_ਸੁਨੇਹਾ_LOADING ||
	     event->type == CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ ||
	     event->type == CONTENT_ਸੁਨੇਹਾ_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->ਅਧਾਰ, false, c->ਅਧਾਰ.ਉਪਲਬਧ_ਚੌੜਾਈ,
				c->ਅਧਾਰ.ਉਪਲਬਧ_ਉਚਾਈ);
		content_ਸੈੱਟ_ਪੂਰਾ(&c->ਅਧਾਰ);
	} ਨਹੀਂਤਾਂ ਜੇ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ &&
		   box != ਨਿਰਾ &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->ਅਧਾਰ.ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
		    c->ਅਧਾਰ.ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		ਜੇ (ms_now > c->ਅਧਾਰ.reformat_ਸਮਾਂ) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->ਅਧਾਰ,
					  false,
					  c->ਅਧਾਰ.ਉਪਲਬਧ_ਚੌੜਾਈ,
					  c->ਅਧਾਰ.ਉਪਲਬਧ_ਉਚਾਈ);
		}
	}

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ਸਥਿਰ bool html_replace_object(ਬਣਤਰ content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;
	html_content *ਪੰਨਾ;
	nserror error;

	ਪ੍ਰਤਿਜ੍ਞਾ(object != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(object->box != ਨਿਰਾ);

	c = (html_content *) object->ਮਾਪਾ;

	ਬੱਚਾ.charset = c->encoding;
	ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

	ਜੇ (object->content != ਨਿਰਾ) {
		/* remove existing object */
		ਜੇ (content_get_ਸਥਿਤੀ(object->content) != CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
			c->ਅਧਾਰ.ਸਰਗਰਮ--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->ਅਧਾਰ.ਸਰਗਰਮ);
		}

		hlcache_handle_release(object->content);
		object->content = ਨਿਰਾ;

		object->box->object = ਨਿਰਾ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
			html_object_callback, object, &ਬੱਚਾ,
			object->permitted_types,
			&object->content);

	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ false;

	ਲਈ (ਪੰਨਾ = c; ਪੰਨਾ != ਨਿਰਾ; ਪੰਨਾ = ਪੰਨਾ->ਪੰਨਾ) {
		ਪੰਨਾ->ਅਧਾਰ.ਸਰਗਰਮ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

		ਪੰਨਾ->ਅਧਾਰ.ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_READY;
	}

	ਵਾਪਸ true;
}

/**
 * schedule callback for object refresh
 */
ਸਥਿਰ ਖਾਲੀ html_object_refresh(ਖਾਲੀ *p)
{
	ਬਣਤਰ content_html_object *object = p;
	nsurl *refresh_url;

	ਪ੍ਰਤਿਜ੍ਞਾ(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ਜੇ (refresh_url == ਨਿਰਾ)
		ਵਾਪਸ;

	content_invalidate_reuse_ਅੰਕੜੇ(object->content);

	ਜੇ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ਬਣਤਰ browser_ਖਿੜਕੀ *bw)
{
	ਬਣਤਰ content_html_object *object, *ਅਗਲਾ;

	ਲਈ (object = html->object_list; object != ਨਿਰਾ; object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		ਜੇ (object->content == ਨਿਰਾ || object->box == ਨਿਰਾ)
			ਜਾਰੀ;

		ਜੇ (content_get_type(object->content) == CONTENT_NONE)
			ਜਾਰੀ;

		content_open(object->content,
			     bw,
			     &html->ਅਧਾਰ,
			     object->box->object_params);
	}
	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ਬਣਤਰ content_html_object *object;

	ਲਈ (object = htmlc->object_list;
	     object != ਨਿਰਾ;
	     object = object->ਅਗਲਾ) {
		ਜੇ (object->content == ਨਿਰਾ)
			ਜਾਰੀ;

		ਚੋਣ (content_get_ਸਥਿਤੀ(object->content)) {
		ਹਾਲਤ CONTENT_ਸਥਿਤੀ_ਪੂਰਾ:
			/* already loaded: do nothing */
			ਰੋਕੋ;

		ਹਾਲਤ CONTENT_ਸਥਿਤੀ_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ਰੋਕੋ;

		ਮੂਲ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ਨਿਰਾ;
			ਜੇ (object->box != ਨਿਰਾ) {
				htmlc->ਅਧਾਰ.ਸਰਗਰਮ--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->ਅਧਾਰ.ਸਰਗਰਮ);
			}
			ਰੋਕੋ;

		}
	}

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ਬਣਤਰ content_html_object *object, *ਅਗਲਾ;

	ਲਈ (object = html->object_list; object != ਨਿਰਾ; object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		ਜੇ (object->content == ਨਿਰਾ || object->box == ਨਿਰਾ)
			ਜਾਰੀ;

		ਜੇ (content_get_type(object->content) == CONTENT_NONE)
			ਜਾਰੀ;

		ਜੇ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ਜਦੋਂ (html->object_list != ਨਿਰਾ) {
		ਬਣਤਰ content_html_object *victim = html->object_list;

		ਜੇ (victim->content != ਨਿਰਾ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ਜੇ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->ਅਗਲਾ;
		ਮੁਕਤ(victim);
	}
	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ਬਣਤਰ box *box,
		  content_type permitted_types,
		  bool background)
{
	ਬਣਤਰ content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ਜੇ (c->aborted)
		ਵਾਪਸ true;

	ਬੱਚਾ.charset = c->encoding;
	ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

	object = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content_html_object));
	ਜੇ (object == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	ਜੇ (box == ਨਿਰਾ) {
		object_callback = html_object_nobox_callback;
	} ਨਹੀਂਤਾਂ {
		object_callback = html_object_callback;
	}

	object->ਮਾਪਾ = (ਬਣਤਰ content *) c;
	object->ਅਗਲਾ = ਨਿਰਾ;
	object->content = ਨਿਰਾ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->ਅਧਾਰ),
					ਨਿਰਾ,
					object_callback,
					object,
					&ਬੱਚਾ,
					object->permitted_types,
					&object->content);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(object);
		ਵਾਪਸ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->ਅਗਲਾ = c->object_list;
	c->object_list = object;

	c->ਅੰਕ_objects++;
	ਜੇ (box != ਨਿਰਾ) {
		c->ਅਧਾਰ.ਸਰਗਰਮ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);
	}

	ਵਾਪਸ true;
}
