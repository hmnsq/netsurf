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

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <nsutils/time.h>

#include "utils/corestrings.h"
#include "utils/config.h"
#include "utils/log.h"
#include "utils/nsoption.h"
#include "netsurf/content.h"
#include "netsurf/misc.h"
#include "content/hlcache.h"
#include "css/utils.h"
#include "desktop/scrollbar.h"
#include "desktop/gui_internal.h"

#include "html/html.h"
#include "html/private.h"
#include "html/interaction.h"
#include "html/box.h"
#include "html/box_inspect.h"
#include "html/object.h"

/* break reference loop */
static void html_object_refresh(void *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
sa_nracanaa content_html_object *html_get_objects(hlcache_handle *h, unsigned puur_naa_nka *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->num_objects;

	ᬯᬮᬶ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

static void
html_object_failed(sa_nracanaa box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ᬯᬮᬶ;
}

/**
 * Update a box whose content has completed rendering.
 */

static void
html_object_done(sa_nracanaa box *box,
		 hlcache_handle *object,
		 bool background)
{
	sa_nracanaa box *b;

	ᬬᬾᬦ᭄ (background) {
		box->background = object;
		ᬯᬮᬶ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	switch (box->type) {
	pak_sa BOX_TABLE:
		box->type = BOX_BLOCK;
		bha_nga;
	default:
		/* TODO: Any other box types need mapping? */
		bha_nga;
	}

	ᬬᬾᬦ᭄ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		for (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		ᬲᭂᬤᭂᬮ (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
static nserror
html_object_nobox_callback(hlcache_handle *object,
			   const hlcache_event *event,
			   void *pw)
{
	sa_nracanaa content_html_object *chobject = pw;

	switch (event->type) {
	pak_sa CONTENT_MSG_ERROR:
		hlcache_handle_release(object);

		chobject->content = NULL;
		bha_nga;

	default:
		bha_nga;
	}

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
static nserror
html_object_callback(hlcache_handle *object,
		     const hlcache_event *event,
		     void *pw)
{
	sa_nracanaa content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	puur_naa_nka x, y;
	sa_nracanaa box *box;

	box = o->box;

	switch (event->type) {
	pak_sa CONTENT_MSG_LOADING:
		ᬬᬾᬦ᭄ (c->base.status != CONTENT_STATUS_LOADING && c->bw != NULL)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		bha_nga;

	pak_sa CONTENT_MSG_READY:
		ᬬᬾᬦ᭄ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			ᬬᬾᬦ᭄ (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		bha_nga;

	pak_sa CONTENT_MSG_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		ᬬᬾᬦ᭄ (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			sa_ngha content_msg_data data;

			ᬬᬾᬦ᭄ (c->had_initial_layout == false) {
				bha_nga;
			}

			ᬬᬾᬦ᭄ (!box_visible(box))
				bha_nga;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[LEFT];
			data.redraw.y = y + box->padding[TOP];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_MSG_REDRAW, &data);
		}
		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		hlcache_handle_release(object);

		o->content = NULL;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		bha_nga;

	pak_sa CONTENT_MSG_REDRAW:
		ᬬᬾᬦ᭄ (c->base.status != CONTENT_STATUS_LOADING) {
			sa_ngha content_msg_data data = event->data;

			ᬬᬾᬦ᭄ (c->had_initial_layout == false) {
				bha_nga;
			}

			ᬬᬾᬦ᭄ (!box_visible(box))
				bha_nga;

			box_coords(box, &x, &y);

			ᬬᬾᬦ᭄ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				puur_naa_nka width = box->padding[LEFT] + box->width +
						box->padding[RIGHT];
				puur_naa_nka height = box->padding[TOP] + box->height +
						box->padding[BOTTOM];
				puur_naa_nka t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				ᬬᬾᬦ᭄ (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} ᬮᬶᬬ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				ᬬᬾᬦ᭄ (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} ᬮᬶᬬ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				switch (css_computed_background_repeat(
						box->style)) {
				pak_sa CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					bha_nga;

				pak_sa CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					bha_nga;

				pak_sa CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					bha_nga;

				pak_sa CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					bha_nga;

				default:
					bha_nga;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} ᬮᬶᬬ {
				/* Non-background case */
				puur_naa_nka w = content_get_width(object);
				puur_naa_nka h = content_get_height(object);

				ᬬᬾᬦ᭄ (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				ᬬᬾᬦ᭄ (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[LEFT];
				data.redraw.y += y + box->padding[TOP];
			}

			content_broadcast(&c->base, CONTENT_MSG_REDRAW, &data);
		}
		bha_nga;

	pak_sa CONTENT_MSG_REFRESH:
		ᬬᬾᬦ᭄ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		bha_nga;

	pak_sa CONTENT_MSG_LINK:
		/* Don't care about favicons that aren't on top level content */
		bha_nga;

	pak_sa CONTENT_MSG_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = NULL;
		bha_nga;

	pak_sa CONTENT_MSG_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		bha_nga;

	pak_sa CONTENT_MSG_SCROLL:
		ᬬᬾᬦ᭄ (box->scroll_x != NULL)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		ᬬᬾᬦ᭄ (box->scroll_y != NULL)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		bha_nga;

	pak_sa CONTENT_MSG_DRAGSAVE:
	{
		sa_ngha content_msg_data msg_data;
		ᬬᬾᬦ᭄ (event->data.dragsave.content == NULL)
			msg_data.dragsave.content = object;
		ᬮᬶᬬ
			msg_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_MSG_DRAGSAVE, &msg_data);
	}
		bha_nga;

	pak_sa CONTENT_MSG_SAVELINK:
	pak_sa CONTENT_MSG_POINTER:
	pak_sa CONTENT_MSG_SELECTMENU:
	pak_sa CONTENT_MSG_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		bha_nga;

	pak_sa CONTENT_MSG_CARET:
	{
		sa_ngha html_focus_owner focus_owner;
		focus_owner.content = box;

		switch (event->data.caret.type) {
		pak_sa CONTENT_CARET_REMOVE:
		pak_sa CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, NULL);
			bha_nga;
		pak_sa CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			bha_nga;
		}
	}
		bha_nga;

	pak_sa CONTENT_MSG_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		sa_ngha html_drag_owner drag_owner;
		drag_owner.content = box;

		switch (event->data.drag.type) {
		pak_sa CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			bha_nga;
		pak_sa CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			bha_nga;
		pak_sa CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			bha_nga;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		bha_nga;

	pak_sa CONTENT_MSG_SELECTION:
	{
		html_selection_type sel_type;
		sa_ngha html_selection_owner sel_owner;

		ᬬᬾᬦ᭄ (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ᬮᬶᬬ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		bha_nga;

	default:
		bha_nga;
	}

	ᬬᬾᬦ᭄ (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_MSG_LOADING ||
	     event->type == CONTENT_MSG_DONE ||
	     event->type == CONTENT_MSG_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} ᬮᬶᬬ ᬬᬾᬦ᭄ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_MSG_DONE &&
		   box != NULL &&
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
		ᬬᬾᬦ᭄ (ms_now > c->base.reformat_time) {
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

	ᬯᬮᬶ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
static bool html_replace_object(sa_nracanaa content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	assert(object != NULL);
	assert(object->box != NULL);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ᬬᬾᬦ᭄ (object->content != NULL) {
		/* remove existing object */
		ᬬᬾᬦ᭄ (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = NULL;

		object->box->object = NULL;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), NULL,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	ᬬᬾᬦ᭄ (error != NSERROR_OK)
		ᬯᬮᬶ false;

	for (page = c; page != NULL; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	ᬯᬮᬶ true;
}

/**
 * schedule callback for object refresh
 */
static void html_object_refresh(void *p)
{
	sa_nracanaa content_html_object *object = p;
	nsurl *refresh_url;

	assert(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ᬬᬾᬦ᭄ (refresh_url == NULL)
		ᬯᬮᬶ;

	content_invalidate_reuse_data(object->content);

	ᬬᬾᬦ᭄ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, sa_nracanaa browser_window *bw)
{
	sa_nracanaa content_html_object *object, *next;

	for (object = html->object_list; object != NULL; object = next) {
		next = object->next;

		ᬬᬾᬦ᭄ (object->content == NULL || object->box == NULL)
			continue;

		ᬬᬾᬦ᭄ (content_get_type(object->content) == CONTENT_NONE)
			continue;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	ᬯᬮᬶ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	sa_nracanaa content_html_object *object;

	for (object = htmlc->object_list;
	     object != NULL;
	     object = object->next) {
		ᬬᬾᬦ᭄ (object->content == NULL)
			continue;

		switch (content_get_status(object->content)) {
		pak_sa CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			bha_nga;

		pak_sa CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			bha_nga;

		default:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = NULL;
			ᬬᬾᬦ᭄ (object->box != NULL) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			bha_nga;

		}
	}

	ᬯᬮᬶ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	sa_nracanaa content_html_object *object, *next;

	for (object = html->object_list; object != NULL; object = next) {
		next = object->next;

		ᬬᬾᬦ᭄ (object->content == NULL || object->box == NULL)
			continue;

		ᬬᬾᬦ᭄ (content_get_type(object->content) == CONTENT_NONE)
			continue;

		ᬬᬾᬦ᭄ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ᬯᬮᬶ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ᬲᭂᬤᭂᬮ (html->object_list != NULL) {
		sa_nracanaa content_html_object *victim = html->object_list;

		ᬬᬾᬦ᭄ (victim->content != NULL) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ᬬᬾᬦ᭄ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		free(victim);
	}
	ᬯᬮᬶ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  sa_nracanaa box *box,
		  content_type permitted_types,
		  bool background)
{
	sa_nracanaa content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ᬬᬾᬦ᭄ (c->aborted)
		ᬯᬮᬶ true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, parimaa_na(sa_nracanaa content_html_object));
	ᬬᬾᬦ᭄ (object == NULL) {
		ᬯᬮᬶ false;
	}

	ᬬᬾᬦ᭄ (box == NULL) {
		object_callback = html_object_nobox_callback;
	} ᬮᬶᬬ {
		object_callback = html_object_callback;
	}

	object->parent = (sa_nracanaa content *) c;
	object->next = NULL;
	object->content = NULL;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					NULL,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(object);
		ᬯᬮᬶ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->num_objects++;
	ᬬᬾᬦ᭄ (box != NULL) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ᬯᬮᬶ true;
}
