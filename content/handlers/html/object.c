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

#شامل <assert.h>
#شامل <ctype.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdlib.h>
#شامل <nsutils/time.h>

#شامل "utils/corestrings.h"
#شامل "utils/config.h"
#شامل "utils/log.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/content.h"
#شامل "netsurf/misc.h"
#شامل "content/hlcache.h"
#شامل "css/utils.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/gui_internal.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/object.h"

/* break reference loop */
ایستا تهی html_object_refresh(تهی *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ساختار content_html_object *html_get_objects(hlcache_handle *h, unsigned صحیح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);
	أكد(n != پوچ);

	*n = c->رقم_objects;

	بازگشت c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ایستا تهی
html_object_failed(ساختار box *box, html_content *content, bool background)
{
	/* Nothing to do */
	بازگشت;
}

/**
 * Update a box whose content has completed rendering.
 */

ایستا تهی
html_object_done(ساختار box *box,
		 hlcache_handle *object,
		 bool background)
{
	ساختار box *b;

	اگر (background) {
		box->background = object;
		بازگشت;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	انتخاب (box->type) {
	حالت BOX_TABLE:
		box->type = BOX_BLOCK;
		بشکن;
	پیشفرض:
		/* TODO: Any other box types need mapping? */
		بشکن;
	}

	اگر (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		برای (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		وقتی (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ایستا nserror
html_object_nobox_callback(hlcache_handle *object,
			   ثابت hlcache_event *event,
			   تهی *pw)
{
	ساختار content_html_object *chobject = pw;

	انتخاب (event->type) {
	حالت CONTENT_پیام_ERROR:
		hlcache_handle_release(object);

		chobject->content = پوچ;
		بشکن;

	پیشفرض:
		بشکن;
	}

	بازگشت NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ایستا nserror
html_object_callback(hlcache_handle *object,
		     ثابت hlcache_event *event,
		     تهی *pw)
{
	ساختار content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	صحیح x, y;
	ساختار box *box;

	box = o->box;

	انتخاب (event->type) {
	حالت CONTENT_پیام_LOADING:
		اگر (c->base.status != CONTENT_STATUS_LOADING && c->bw != پوچ)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		بشکن;

	حالت CONTENT_پیام_READY:
		اگر (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			اگر (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		بشکن;

	حالت CONTENT_پیام_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		اگر (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			اجتماع content_پیام_data data;

			اگر (c->had_initial_layout == false) {
				بشکن;
			}

			اگر (!box_visible(box))
				بشکن;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[چپ_2];
			data.redraw.y = y + box->padding[بالاترین_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_پیام_REDRAW, &data);
		}
		بشکن;

	حالت CONTENT_پیام_ERROR:
		hlcache_handle_release(object);

		o->content = پوچ;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		بشکن;

	حالت CONTENT_پیام_REDRAW:
		اگر (c->base.status != CONTENT_STATUS_LOADING) {
			اجتماع content_پیام_data data = event->data;

			اگر (c->had_initial_layout == false) {
				بشکن;
			}

			اگر (!box_visible(box))
				بشکن;

			box_coords(box, &x, &y);

			اگر (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				صحیح width = box->padding[چپ_2] + box->width +
						box->padding[راست_2];
				صحیح height = box->padding[بالاترین_2] + box->height +
						box->padding[BOTTOM];
				صحیح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				اگر (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} وگرنه {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				اگر (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} وگرنه {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				انتخاب (css_computed_background_repeat(
						box->style)) {
				حالت CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					بشکن;

				حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					بشکن;

				حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					بشکن;

				حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					بشکن;

				پیشفرض:
					بشکن;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} وگرنه {
				/* Non-background case */
				صحیح w = content_get_width(object);
				صحیح h = content_get_height(object);

				اگر (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				اگر (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[چپ_2];
				data.redraw.y += y + box->padding[بالاترین_2];
			}

			content_broadcast(&c->base, CONTENT_پیام_REDRAW, &data);
		}
		بشکن;

	حالت CONTENT_پیام_REFRESH:
		اگر (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		بشکن;

	حالت CONTENT_پیام_LINK:
		/* Don't care about favicons that aren't on top level content */
		بشکن;

	حالت CONTENT_پیام_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = پوچ;
		بشکن;

	حالت CONTENT_پیام_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		بشکن;

	حالت CONTENT_پیام_SCROLL:
		اگر (box->scroll_x != پوچ)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		اگر (box->scroll_y != پوچ)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		بشکن;

	حالت CONTENT_پیام_DRAGSAVE:
	{
		اجتماع content_پیام_data پیام_data;
		اگر (event->data.dragsave.content == پوچ)
			پیام_data.dragsave.content = object;
		وگرنه
			پیام_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_پیام_DRAGSAVE, &پیام_data);
	}
		بشکن;

	حالت CONTENT_پیام_SAVELINK:
	حالت CONTENT_پیام_POINTER:
	حالت CONTENT_پیام_SELECTMENU:
	حالت CONTENT_پیام_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		بشکن;

	حالت CONTENT_پیام_CARET:
	{
		اجتماع html_focus_owner focus_owner;
		focus_owner.content = box;

		انتخاب (event->data.caret.type) {
		حالت CONTENT_CARET_REMOVE:
		حالت CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, پوچ);
			بشکن;
		حالت CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			بشکن;
		}
	}
		بشکن;

	حالت CONTENT_پیام_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اجتماع html_drag_owner drag_owner;
		drag_owner.content = box;

		انتخاب (event->data.drag.type) {
		حالت CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			بشکن;
		حالت CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			بشکن;
		حالت CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			بشکن;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		بشکن;

	حالت CONTENT_پیام_SELECTION:
	{
		html_selection_type sel_type;
		اجتماع html_selection_owner sel_owner;

		اگر (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} وگرنه {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		بشکن;

	پیشفرض:
		بشکن;
	}

	اگر (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_پیام_LOADING ||
	     event->type == CONTENT_پیام_DONE ||
	     event->type == CONTENT_پیام_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} وگرنه اگر (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_پیام_DONE &&
		   box != پوچ &&
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
		اگر (ms_now > c->base.reformat_time) {
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

	بازگشت NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ایستا bool html_replace_object(ساختار content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	أكد(object != پوچ);
	أكد(object->box != پوچ);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	اگر (object->content != پوچ) {
		/* remove existing object */
		اگر (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = پوچ;

		object->box->object = پوچ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), پوچ,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	اگر (error != NSERROR_OK)
		بازگشت false;

	برای (page = c; page != پوچ; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	بازگشت true;
}

/**
 * schedule callback for object refresh
 */
ایستا تهی html_object_refresh(تهی *p)
{
	ساختار content_html_object *object = p;
	nsurl *refresh_url;

	أكد(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	اگر (refresh_url == پوچ)
		بازگشت;

	content_invalidate_reuse_data(object->content);

	اگر (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ساختار browser_window *bw)
{
	ساختار content_html_object *object, *next;

	برای (object = html->object_list; object != پوچ; object = next) {
		next = object->next;

		اگر (object->content == پوچ || object->box == پوچ)
			ادامه;

		اگر (content_get_type(object->content) == CONTENT_NONE)
			ادامه;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	بازگشت NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ساختار content_html_object *object;

	برای (object = htmlc->object_list;
	     object != پوچ;
	     object = object->next) {
		اگر (object->content == پوچ)
			ادامه;

		انتخاب (content_get_status(object->content)) {
		حالت CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			بشکن;

		حالت CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			بشکن;

		پیشفرض:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = پوچ;
			اگر (object->box != پوچ) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			بشکن;

		}
	}

	بازگشت NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ساختار content_html_object *object, *next;

	برای (object = html->object_list; object != پوچ; object = next) {
		next = object->next;

		اگر (object->content == پوچ || object->box == پوچ)
			ادامه;

		اگر (content_get_type(object->content) == CONTENT_NONE)
			ادامه;

		اگر (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	بازگشت NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	وقتی (html->object_list != پوچ) {
		ساختار content_html_object *victim = html->object_list;

		اگر (victim->content != پوچ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			اگر (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		آزاد(victim);
	}
	بازگشت NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ساختار box *box,
		  content_type permitted_types,
		  bool background)
{
	ساختار content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	اگر (c->aborted)
		بازگشت true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, اندازه(ساختار content_html_object));
	اگر (object == پوچ) {
		بازگشت false;
	}

	اگر (box == پوچ) {
		object_callback = html_object_nobox_callback;
	} وگرنه {
		object_callback = html_object_callback;
	}

	object->parent = (ساختار content *) c;
	object->next = پوچ;
	object->content = پوچ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					پوچ,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	اگر (error != NSERROR_OK) {
		آزاد(object);
		بازگشت error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->رقم_objects++;
	اگر (box != پوچ) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	بازگشت true;
}
