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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <nsutils/time.h>

#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "css/utils.h"
#অন্তর্ভাব "desktop/scrollbar.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/interaction.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/object.h"

/* break reference loop */
স্থির শূন্য html_object_refresh(শূন্য *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
সংরচনা content_html_object *html_get_objects(hlcache_handle *h, unsigned পূর্ণাঙ্ক *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);
	প্রতিজ্ঞা(n != রিক্ত);

	*n = c->অঙ্ক_objects;

	প্রত্যাগম c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

স্থির শূন্য
html_object_failed(সংরচনা box *box, html_content *content, bool background)
{
	/* Nothing to do */
	প্রত্যাগম;
}

/**
 * Update a box whose content has completed rendering.
 */

স্থির শূন্য
html_object_done(সংরচনা box *box,
		 hlcache_handle *object,
		 bool background)
{
	সংরচনা box *b;

	যদি (background) {
		box->background = object;
		প্রত্যাগম;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	বিকল্প (box->type) {
	পক্ষ BOX_TABLE:
		box->type = BOX_BLOCK;
		ভঙ্গ;
	স্বভাব:
		/* TODO: Any other box types need mapping? */
		ভঙ্গ;
	}

	যদি (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		প্রতি (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		যাবত্ (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
স্থির nserror
html_object_nobox_callback(hlcache_handle *object,
			   ধ্রুব hlcache_event *event,
			   শূন্য *pw)
{
	সংরচনা content_html_object *chobject = pw;

	বিকল্প (event->type) {
	পক্ষ CONTENT_বার্তা_ERROR:
		hlcache_handle_release(object);

		chobject->content = রিক্ত;
		ভঙ্গ;

	স্বভাব:
		ভঙ্গ;
	}

	প্রত্যাগম NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
স্থির nserror
html_object_callback(hlcache_handle *object,
		     ধ্রুব hlcache_event *event,
		     শূন্য *pw)
{
	সংরচনা content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	পূর্ণাঙ্ক x, y;
	সংরচনা box *box;

	box = o->box;

	বিকল্প (event->type) {
	পক্ষ CONTENT_বার্তা_LOADING:
		যদি (c->base.status != CONTENT_STATUS_LOADING && c->bw != রিক্ত)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_READY:
		যদি (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			যদি (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		যদি (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			সঙ্ঘ content_বার্তা_data data;

			যদি (c->had_initial_layout == false) {
				ভঙ্গ;
			}

			যদি (!box_visible(box))
				ভঙ্গ;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[বাম_2];
			data.redraw.y = y + box->padding[শীর্ষ_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_বার্তা_REDRAW, &data);
		}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_ERROR:
		hlcache_handle_release(object);

		o->content = রিক্ত;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_REDRAW:
		যদি (c->base.status != CONTENT_STATUS_LOADING) {
			সঙ্ঘ content_বার্তা_data data = event->data;

			যদি (c->had_initial_layout == false) {
				ভঙ্গ;
			}

			যদি (!box_visible(box))
				ভঙ্গ;

			box_coords(box, &x, &y);

			যদি (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				পূর্ণাঙ্ক width = box->padding[বাম_2] + box->width +
						box->padding[ডান_2];
				পূর্ণাঙ্ক height = box->padding[শীর্ষ_2] + box->height +
						box->padding[BOTTOM];
				পূর্ণাঙ্ক t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				যদি (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} অন্যথা {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				যদি (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} অন্যথা {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				বিকল্প (css_computed_background_repeat(
						box->style)) {
				পক্ষ CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					ভঙ্গ;

				পক্ষ CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					ভঙ্গ;

				পক্ষ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					ভঙ্গ;

				পক্ষ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					ভঙ্গ;

				স্বভাব:
					ভঙ্গ;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} অন্যথা {
				/* Non-background case */
				পূর্ণাঙ্ক w = content_get_width(object);
				পূর্ণাঙ্ক h = content_get_height(object);

				যদি (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				যদি (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[বাম_2];
				data.redraw.y += y + box->padding[শীর্ষ_2];
			}

			content_broadcast(&c->base, CONTENT_বার্তা_REDRAW, &data);
		}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_REFRESH:
		যদি (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_LINK:
		/* Don't care about favicons that aren't on top level content */
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = রিক্ত;
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_SCROLL:
		যদি (box->scroll_x != রিক্ত)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		যদি (box->scroll_y != রিক্ত)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_DRAGSAVE:
	{
		সঙ্ঘ content_বার্তা_data বার্তা_data;
		যদি (event->data.dragsave.content == রিক্ত)
			বার্তা_data.dragsave.content = object;
		অন্যথা
			বার্তা_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_বার্তা_DRAGSAVE, &বার্তা_data);
	}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_SAVELINK:
	পক্ষ CONTENT_বার্তা_POINTER:
	পক্ষ CONTENT_বার্তা_SELECTMENU:
	পক্ষ CONTENT_বার্তা_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_CARET:
	{
		সঙ্ঘ html_focus_owner focus_owner;
		focus_owner.content = box;

		বিকল্প (event->data.caret.type) {
		পক্ষ CONTENT_CARET_REMOVE:
		পক্ষ CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, রিক্ত);
			ভঙ্গ;
		পক্ষ CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			ভঙ্গ;
		}
	}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		সঙ্ঘ html_drag_owner drag_owner;
		drag_owner.content = box;

		বিকল্প (event->data.drag.type) {
		পক্ষ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ভঙ্গ;
		পক্ষ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ভঙ্গ;
		পক্ষ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ভঙ্গ;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_SELECTION:
	{
		html_selection_type sel_type;
		সঙ্ঘ html_selection_owner sel_owner;

		যদি (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} অন্যথা {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		ভঙ্গ;

	স্বভাব:
		ভঙ্গ;
	}

	যদি (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_বার্তা_LOADING ||
	     event->type == CONTENT_বার্তা_DONE ||
	     event->type == CONTENT_বার্তা_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} অন্যথা যদি (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_বার্তা_DONE &&
		   box != রিক্ত &&
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
		যদি (ms_now > c->base.reformat_time) {
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

	প্রত্যাগম NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
স্থির bool html_replace_object(সংরচনা content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	প্রতিজ্ঞা(object != রিক্ত);
	প্রতিজ্ঞা(object->box != রিক্ত);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	যদি (object->content != রিক্ত) {
		/* remove existing object */
		যদি (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = রিক্ত;

		object->box->object = রিক্ত;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), রিক্ত,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	যদি (error != NSERROR_OK)
		প্রত্যাগম false;

	প্রতি (page = c; page != রিক্ত; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	প্রত্যাগম true;
}

/**
 * schedule callback for object refresh
 */
স্থির শূন্য html_object_refresh(শূন্য *p)
{
	সংরচনা content_html_object *object = p;
	nsurl *refresh_url;

	প্রতিজ্ঞা(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	যদি (refresh_url == রিক্ত)
		প্রত্যাগম;

	content_invalidate_reuse_data(object->content);

	যদি (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, সংরচনা browser_window *bw)
{
	সংরচনা content_html_object *object, *next;

	প্রতি (object = html->object_list; object != রিক্ত; object = next) {
		next = object->next;

		যদি (object->content == রিক্ত || object->box == রিক্ত)
			অনুবর্ত;

		যদি (content_get_type(object->content) == CONTENT_NONE)
			অনুবর্ত;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	সংরচনা content_html_object *object;

	প্রতি (object = htmlc->object_list;
	     object != রিক্ত;
	     object = object->next) {
		যদি (object->content == রিক্ত)
			অনুবর্ত;

		বিকল্প (content_get_status(object->content)) {
		পক্ষ CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			ভঙ্গ;

		পক্ষ CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ভঙ্গ;

		স্বভাব:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = রিক্ত;
			যদি (object->box != রিক্ত) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			ভঙ্গ;

		}
	}

	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	সংরচনা content_html_object *object, *next;

	প্রতি (object = html->object_list; object != রিক্ত; object = next) {
		next = object->next;

		যদি (object->content == রিক্ত || object->box == রিক্ত)
			অনুবর্ত;

		যদি (content_get_type(object->content) == CONTENT_NONE)
			অনুবর্ত;

		যদি (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	যাবত্ (html->object_list != রিক্ত) {
		সংরচনা content_html_object *victim = html->object_list;

		যদি (victim->content != রিক্ত) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			যদি (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		মুক্ত(victim);
	}
	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  সংরচনা box *box,
		  content_type permitted_types,
		  bool background)
{
	সংরচনা content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	যদি (c->aborted)
		প্রত্যাগম true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, পরিমাণ(সংরচনা content_html_object));
	যদি (object == রিক্ত) {
		প্রত্যাগম false;
	}

	যদি (box == রিক্ত) {
		object_callback = html_object_nobox_callback;
	} অন্যথা {
		object_callback = html_object_callback;
	}

	object->parent = (সংরচনা content *) c;
	object->next = রিক্ত;
	object->content = রিক্ত;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					রিক্ত,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	যদি (error != NSERROR_OK) {
		মুক্ত(object);
		প্রত্যাগম error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->অঙ্ক_objects++;
	যদি (box != রিক্ত) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	প্রত্যাগম true;
}
