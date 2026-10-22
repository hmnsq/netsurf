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
ಸಂರಚನಾ content_html_object *html_get_objects(hlcache_handle *h, unsigned ಪೂರ್ಣಾಙ್ಕ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(n != ರಿಕ್ತ);

	*n = c->ಅಂಕೆ_objects;

	ಪ್ರತ್ಯಾಗಮ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_object_failed(ಸಂರಚನಾ box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ಪ್ರತ್ಯಾಗಮ;
}

/**
 * Update a box whose content has completed rendering.
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_object_done(ಸಂರಚನಾ box *box,
		 hlcache_handle *object,
		 bool background)
{
	ಸಂರಚನಾ box *b;

	ಯದಿ (background) {
		box->background = object;
		ಪ್ರತ್ಯಾಗಮ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	ವಿಕಲ್ಪ (box->type) {
	ಪಕ್ಷ BOX_TABLE:
		box->type = BOX_BLOCK;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* TODO: Any other box types need mapping? */
		ಭಙ್ಗ;
	}

	ಯದಿ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		ಪ್ರತಿ (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		ಯಾವತ್ (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ಸ್ಥಿರ nserror
html_object_nobox_callback(hlcache_handle *object,
			   ಧ್ರುವ hlcache_event *event,
			   ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content_html_object *chobject = pw;

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(object);

		chobject->content = ರಿಕ್ತ;
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ಸ್ಥಿರ nserror
html_object_callback(hlcache_handle *object,
		     ಧ್ರುವ hlcache_event *event,
		     ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	ಪೂರ್ಣಾಙ್ಕ x, y;
	ಸಂರಚನಾ box *box;

	box = o->box;

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_LOADING:
		ಯದಿ (c->base.status != CONTENT_STATUS_LOADING && c->bw != ರಿಕ್ತ)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_READY:
		ಯದಿ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			ಯದಿ (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		ಯದಿ (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			ಸಙ್ಘ content_ಸಂದೇಶ_data data;

			ಯದಿ (c->had_initial_layout == false) {
				ಭಙ್ಗ;
			}

			ಯದಿ (!box_visible(box))
				ಭಙ್ಗ;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[ಎಡ_2];
			data.redraw.y = y + box->padding[ತುದಿ_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_ಸಂದೇಶ_REDRAW, &data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(object);

		o->content = ರಿಕ್ತ;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_REDRAW:
		ಯದಿ (c->base.status != CONTENT_STATUS_LOADING) {
			ಸಙ್ಘ content_ಸಂದೇಶ_data data = event->data;

			ಯದಿ (c->had_initial_layout == false) {
				ಭಙ್ಗ;
			}

			ಯದಿ (!box_visible(box))
				ಭಙ್ಗ;

			box_coords(box, &x, &y);

			ಯದಿ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				ಪೂರ್ಣಾಙ್ಕ width = box->padding[ಎಡ_2] + box->width +
						box->padding[ಬಲ_2];
				ಪೂರ್ಣಾಙ್ಕ height = box->padding[ತುದಿ_2] + box->height +
						box->padding[BOTTOM];
				ಪೂರ್ಣಾಙ್ಕ t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				ಯದಿ (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} ಅನ್ಯಥಾ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				ಯದಿ (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} ಅನ್ಯಥಾ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				ವಿಕಲ್ಪ (css_computed_background_repeat(
						box->style)) {
				ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					ಭಙ್ಗ;

				ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					ಭಙ್ಗ;

				ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					ಭಙ್ಗ;

				ಪಕ್ಷ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					ಭಙ್ಗ;

				ಸ್ವಭಾವ:
					ಭಙ್ಗ;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} ಅನ್ಯಥಾ {
				/* Non-background case */
				ಪೂರ್ಣಾಙ್ಕ w = content_get_width(object);
				ಪೂರ್ಣಾಙ್ಕ h = content_get_height(object);

				ಯದಿ (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				ಯದಿ (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[ಎಡ_2];
				data.redraw.y += y + box->padding[ತುದಿ_2];
			}

			content_broadcast(&c->base, CONTENT_ಸಂದೇಶ_REDRAW, &data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_REFRESH:
		ಯದಿ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_LINK:
		/* Don't care about favicons that aren't on top level content */
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = ರಿಕ್ತ;
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_SCROLL:
		ಯದಿ (box->scroll_x != ರಿಕ್ತ)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		ಯದಿ (box->scroll_y != ರಿಕ್ತ)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DRAGSAVE:
	{
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
		ಯದಿ (event->data.dragsave.content == ರಿಕ್ತ)
			ಸಂದೇಶ_data.dragsave.content = object;
		ಅನ್ಯಥಾ
			ಸಂದೇಶ_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_ಸಂದೇಶ_DRAGSAVE, &ಸಂದೇಶ_data);
	}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_SAVELINK:
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_POINTER:
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_SELECTMENU:
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_CARET:
	{
		ಸಙ್ಘ html_focus_owner focus_owner;
		focus_owner.content = box;

		ವಿಕಲ್ಪ (event->data.caret.type) {
		ಪಕ್ಷ CONTENT_CARET_REMOVE:
		ಪಕ್ಷ CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ರಿಕ್ತ);
			ಭಙ್ಗ;
		ಪಕ್ಷ CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			ಭಙ್ಗ;
		}
	}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		ಸಙ್ಘ html_drag_owner drag_owner;
		drag_owner.content = box;

		ವಿಕಲ್ಪ (event->data.drag.type) {
		ಪಕ್ಷ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ಭಙ್ಗ;
		ಪಕ್ಷ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ಭಙ್ಗ;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_SELECTION:
	{
		html_selection_type sel_type;
		ಸಙ್ಘ html_selection_owner sel_owner;

		ಯದಿ (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ಅನ್ಯಥಾ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಯದಿ (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_ಸಂದೇಶ_LOADING ||
	     event->type == CONTENT_ಸಂದೇಶ_DONE ||
	     event->type == CONTENT_ಸಂದೇಶ_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} ಅನ್ಯಥಾ ಯದಿ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_ಸಂದೇಶ_DONE &&
		   box != ರಿಕ್ತ &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->base.status == CONTENT_STATUS_READY ||
		    c->base.status == CONTENT_STATUS_DONE)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		ಯದಿ (ms_now > c->base.reformat_time) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->base,
					  false,
					  c->base.available_width,
					  c->base.available_height);
		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool html_replace_object(ಸಂರಚನಾ content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(object != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(object->box != ರಿಕ್ತ);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ಯದಿ (object->content != ರಿಕ್ತ) {
		/* remove existing object */
		ಯದಿ (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = ರಿಕ್ತ;

		object->box->object = ರಿಕ್ತ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), ರಿಕ್ತ,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (page = c; page != ರಿಕ್ತ; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * schedule callback for object refresh
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_object_refresh(ಶೂನ್ಯ *p)
{
	ಸಂರಚನಾ content_html_object *object = p;
	nsurl *refresh_url;

	ಪ್ರತಿಜ್ಞಾ(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ಯದಿ (refresh_url == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	content_invalidate_reuse_data(object->content);

	ಯದಿ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ಸಂರಚನಾ browser_window *bw)
{
	ಸಂರಚನಾ content_html_object *object, *next;

	ಪ್ರತಿ (object = html->object_list; object != ರಿಕ್ತ; object = next) {
		next = object->next;

		ಯದಿ (object->content == ರಿಕ್ತ || object->box == ರಿಕ್ತ)
			ಅನುವರ್ತ;

		ಯದಿ (content_get_type(object->content) == CONTENT_NONE)
			ಅನುವರ್ತ;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ಸಂರಚನಾ content_html_object *object;

	ಪ್ರತಿ (object = htmlc->object_list;
	     object != ರಿಕ್ತ;
	     object = object->next) {
		ಯದಿ (object->content == ರಿಕ್ತ)
			ಅನುವರ್ತ;

		ವಿಕಲ್ಪ (content_get_status(object->content)) {
		ಪಕ್ಷ CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			ಭಙ್ಗ;

		ಪಕ್ಷ CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ರಿಕ್ತ;
			ಯದಿ (object->box != ರಿಕ್ತ) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			ಭಙ್ಗ;

		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ಸಂರಚನಾ content_html_object *object, *next;

	ಪ್ರತಿ (object = html->object_list; object != ರಿಕ್ತ; object = next) {
		next = object->next;

		ಯದಿ (object->content == ರಿಕ್ತ || object->box == ರಿಕ್ತ)
			ಅನುವರ್ತ;

		ಯದಿ (content_get_type(object->content) == CONTENT_NONE)
			ಅನುವರ್ತ;

		ಯದಿ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ಯಾವತ್ (html->object_list != ರಿಕ್ತ) {
		ಸಂರಚನಾ content_html_object *victim = html->object_list;

		ಯದಿ (victim->content != ರಿಕ್ತ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ಯದಿ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		ಮುಕ್ತ(victim);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ಸಂರಚನಾ box *box,
		  content_type permitted_types,
		  bool background)
{
	ಸಂರಚನಾ content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ಯದಿ (c->aborted)
		ಪ್ರತ್ಯಾಗಮ true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content_html_object));
	ಯದಿ (object == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (box == ರಿಕ್ತ) {
		object_callback = html_object_nobox_callback;
	} ಅನ್ಯಥಾ {
		object_callback = html_object_callback;
	}

	object->parent = (ಸಂರಚನಾ content *) c;
	object->next = ರಿಕ್ತ;
	object->content = ರಿಕ್ತ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					ರಿಕ್ತ,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(object);
		ಪ್ರತ್ಯಾಗಮ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->ಅಂಕೆ_objects++;
	ಯದಿ (box != ರಿಕ್ತ) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}
