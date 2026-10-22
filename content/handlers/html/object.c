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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <nsutils/time.h>

#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "css/utils.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/interaction.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/object.h"

/* break reference loop */
நிலையான வெறுமை html_object_refresh(வெறுமை *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
கட்டமைப்பு content_html_object *html_get_objects(hlcache_handle *h, unsigned முழு *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);
	ப்ரதிஜ்ஞா(n != வெற்று);

	*n = c->இலக்கம்_objects;

	திரும்பு c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

நிலையான வெறுமை
html_object_failed(கட்டமைப்பு box *box, html_content *content, bool background)
{
	/* Nothing to do */
	திரும்பு;
}

/**
 * Update a box whose content has completed rendering.
 */

நிலையான வெறுமை
html_object_done(கட்டமைப்பு box *box,
		 hlcache_handle *object,
		 bool background)
{
	கட்டமைப்பு box *b;

	எனில் (background) {
		box->background = object;
		திரும்பு;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	தேர்வு (box->type) {
	நிலை BOX_TABLE:
		box->type = BOX_BLOCK;
		நிறுத்து;
	இயல்பு:
		/* TODO: Any other box types need mapping? */
		நிறுத்து;
	}

	எனில் (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		க்கு (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		போது (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
நிலையான nserror
html_object_nobox_callback(hlcache_handle *object,
			   மாறிலி hlcache_event *event,
			   வெறுமை *pw)
{
	கட்டமைப்பு content_html_object *chobject = pw;

	தேர்வு (event->type) {
	நிலை CONTENT_செய்தி_ERROR:
		hlcache_handle_release(object);

		chobject->content = வெற்று;
		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	திரும்பு NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
நிலையான nserror
html_object_callback(hlcache_handle *object,
		     மாறிலி hlcache_event *event,
		     வெறுமை *pw)
{
	கட்டமைப்பு content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	முழு x, y;
	கட்டமைப்பு box *box;

	box = o->box;

	தேர்வு (event->type) {
	நிலை CONTENT_செய்தி_LOADING:
		எனில் (c->base.status != CONTENT_STATUS_LOADING && c->bw != வெற்று)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		நிறுத்து;

	நிலை CONTENT_செய்தி_READY:
		எனில் (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			எனில் (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		நிறுத்து;

	நிலை CONTENT_செய்தி_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		எனில் (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			ஒன்றிப்பு content_செய்தி_data data;

			எனில் (c->had_initial_layout == false) {
				நிறுத்து;
			}

			எனில் (!box_visible(box))
				நிறுத்து;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[இடது_2];
			data.redraw.y = y + box->padding[உச்சி_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_செய்தி_REDRAW, &data);
		}
		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		hlcache_handle_release(object);

		o->content = வெற்று;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		நிறுத்து;

	நிலை CONTENT_செய்தி_REDRAW:
		எனில் (c->base.status != CONTENT_STATUS_LOADING) {
			ஒன்றிப்பு content_செய்தி_data data = event->data;

			எனில் (c->had_initial_layout == false) {
				நிறுத்து;
			}

			எனில் (!box_visible(box))
				நிறுத்து;

			box_coords(box, &x, &y);

			எனில் (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				முழு width = box->padding[இடது_2] + box->width +
						box->padding[வலது_2];
				முழு height = box->padding[உச்சி_2] + box->height +
						box->padding[BOTTOM];
				முழு t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				எனில் (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} இல்லையெனில் {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				எனில் (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} இல்லையெனில் {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				தேர்வு (css_computed_background_repeat(
						box->style)) {
				நிலை CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					நிறுத்து;

				நிலை CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					நிறுத்து;

				நிலை CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					நிறுத்து;

				நிலை CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					நிறுத்து;

				இயல்பு:
					நிறுத்து;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} இல்லையெனில் {
				/* Non-background case */
				முழு w = content_get_width(object);
				முழு h = content_get_height(object);

				எனில் (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				எனில் (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[இடது_2];
				data.redraw.y += y + box->padding[உச்சி_2];
			}

			content_broadcast(&c->base, CONTENT_செய்தி_REDRAW, &data);
		}
		நிறுத்து;

	நிலை CONTENT_செய்தி_REFRESH:
		எனில் (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		நிறுத்து;

	நிலை CONTENT_செய்தி_LINK:
		/* Don't care about favicons that aren't on top level content */
		நிறுத்து;

	நிலை CONTENT_செய்தி_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = வெற்று;
		நிறுத்து;

	நிலை CONTENT_செய்தி_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		நிறுத்து;

	நிலை CONTENT_செய்தி_SCROLL:
		எனில் (box->scroll_x != வெற்று)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		எனில் (box->scroll_y != வெற்று)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		நிறுத்து;

	நிலை CONTENT_செய்தி_DRAGSAVE:
	{
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;
		எனில் (event->data.dragsave.content == வெற்று)
			செய்தி_data.dragsave.content = object;
		இல்லையெனில்
			செய்தி_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_செய்தி_DRAGSAVE, &செய்தி_data);
	}
		நிறுத்து;

	நிலை CONTENT_செய்தி_SAVELINK:
	நிலை CONTENT_செய்தி_POINTER:
	நிலை CONTENT_செய்தி_SELECTMENU:
	நிலை CONTENT_செய்தி_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		நிறுத்து;

	நிலை CONTENT_செய்தி_CARET:
	{
		ஒன்றிப்பு html_focus_owner focus_owner;
		focus_owner.content = box;

		தேர்வு (event->data.caret.type) {
		நிலை CONTENT_CARET_REMOVE:
		நிலை CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, வெற்று);
			நிறுத்து;
		நிலை CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			நிறுத்து;
		}
	}
		நிறுத்து;

	நிலை CONTENT_செய்தி_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		ஒன்றிப்பு html_drag_owner drag_owner;
		drag_owner.content = box;

		தேர்வு (event->data.drag.type) {
		நிலை CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			நிறுத்து;
		நிலை CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			நிறுத்து;
		நிலை CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			நிறுத்து;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		நிறுத்து;

	நிலை CONTENT_செய்தி_SELECTION:
	{
		html_selection_type sel_type;
		ஒன்றிப்பு html_selection_owner sel_owner;

		எனில் (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} இல்லையெனில் {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	எனில் (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_செய்தி_LOADING ||
	     event->type == CONTENT_செய்தி_DONE ||
	     event->type == CONTENT_செய்தி_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} இல்லையெனில் எனில் (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_செய்தி_DONE &&
		   box != வெற்று &&
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
		எனில் (ms_now > c->base.reformat_time) {
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

	திரும்பு NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
நிலையான bool html_replace_object(கட்டமைப்பு content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	ப்ரதிஜ்ஞா(object != வெற்று);
	ப்ரதிஜ்ஞா(object->box != வெற்று);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	எனில் (object->content != வெற்று) {
		/* remove existing object */
		எனில் (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = வெற்று;

		object->box->object = வெற்று;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), வெற்று,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	எனில் (error != NSERROR_OK)
		திரும்பு false;

	க்கு (page = c; page != வெற்று; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	திரும்பு true;
}

/**
 * schedule callback for object refresh
 */
நிலையான வெறுமை html_object_refresh(வெறுமை *p)
{
	கட்டமைப்பு content_html_object *object = p;
	nsurl *refresh_url;

	ப்ரதிஜ்ஞா(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	எனில் (refresh_url == வெற்று)
		திரும்பு;

	content_invalidate_reuse_data(object->content);

	எனில் (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, கட்டமைப்பு browser_window *bw)
{
	கட்டமைப்பு content_html_object *object, *next;

	க்கு (object = html->object_list; object != வெற்று; object = next) {
		next = object->next;

		எனில் (object->content == வெற்று || object->box == வெற்று)
			தொடர்;

		எனில் (content_get_type(object->content) == CONTENT_NONE)
			தொடர்;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	கட்டமைப்பு content_html_object *object;

	க்கு (object = htmlc->object_list;
	     object != வெற்று;
	     object = object->next) {
		எனில் (object->content == வெற்று)
			தொடர்;

		தேர்வு (content_get_status(object->content)) {
		நிலை CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			நிறுத்து;

		நிலை CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			நிறுத்து;

		இயல்பு:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = வெற்று;
			எனில் (object->box != வெற்று) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			நிறுத்து;

		}
	}

	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	கட்டமைப்பு content_html_object *object, *next;

	க்கு (object = html->object_list; object != வெற்று; object = next) {
		next = object->next;

		எனில் (object->content == வெற்று || object->box == வெற்று)
			தொடர்;

		எனில் (content_get_type(object->content) == CONTENT_NONE)
			தொடர்;

		எனில் (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	போது (html->object_list != வெற்று) {
		கட்டமைப்பு content_html_object *victim = html->object_list;

		எனில் (victim->content != வெற்று) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			எனில் (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		விடுவி(victim);
	}
	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  கட்டமைப்பு box *box,
		  content_type permitted_types,
		  bool background)
{
	கட்டமைப்பு content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	எனில் (c->aborted)
		திரும்பு true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, அளவு(கட்டமைப்பு content_html_object));
	எனில் (object == வெற்று) {
		திரும்பு false;
	}

	எனில் (box == வெற்று) {
		object_callback = html_object_nobox_callback;
	} இல்லையெனில் {
		object_callback = html_object_callback;
	}

	object->parent = (கட்டமைப்பு content *) c;
	object->next = வெற்று;
	object->content = வெற்று;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					வெற்று,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	எனில் (error != NSERROR_OK) {
		விடுவி(object);
		திரும்பு error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->இலக்கம்_objects++;
	எனில் (box != வெற்று) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	திரும்பு true;
}
