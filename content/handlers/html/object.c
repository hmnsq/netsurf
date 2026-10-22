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

#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <stdlib.h>
#ضمن <nsutils/time.h>

#ضمن "utils/corestrings.h"
#ضمن "utils/config.h"
#ضمن "utils/log.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/misc.h"
#ضمن "content/hlcache.h"
#ضمن "css/utils.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/interaction.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/object.h"

/* break reference loop */
ساكن فراغ html_object_refresh(فراغ *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
بنية content_html_object *html_get_objects(hlcache_handle *h, unsigned صحيح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->رقم_objects;

	إرجاع c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ساكن فراغ
html_object_failed(بنية box *box, html_content *content, bool background)
{
	/* Nothing to do */
	إرجاع;
}

/**
 * Update a box whose content has completed rendering.
 */

ساكن فراغ
html_object_done(بنية box *box,
		 hlcache_handle *object,
		 bool background)
{
	بنية box *b;

	إذا (background) {
		box->background = object;
		إرجاع;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	بدل (box->type) {
	حالة BOX_TABLE:
		box->type = BOX_BLOCK;
		اكسر;
	افتراضي:
		/* TODO: Any other box types need mapping? */
		اكسر;
	}

	إذا (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		لكل (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		بينما (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ساكن nserror
html_object_nobox_callback(hlcache_handle *object,
			   ثابت hlcache_event *event,
			   فراغ *pw)
{
	بنية content_html_object *chobject = pw;

	بدل (event->type) {
	حالة CONTENT_رسالة_ERROR:
		hlcache_handle_release(object);

		chobject->content = عدم;
		اكسر;

	افتراضي:
		اكسر;
	}

	إرجاع NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ساكن nserror
html_object_callback(hlcache_handle *object,
		     ثابت hlcache_event *event,
		     فراغ *pw)
{
	بنية content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	صحيح x, y;
	بنية box *box;

	box = o->box;

	بدل (event->type) {
	حالة CONTENT_رسالة_LOADING:
		إذا (c->base.status != CONTENT_STATUS_LOADING && c->bw != عدم)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		اكسر;

	حالة CONTENT_رسالة_READY:
		إذا (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			إذا (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		اكسر;

	حالة CONTENT_رسالة_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		إذا (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			اتحاد content_رسالة_data data;

			إذا (c->had_initial_layout == false) {
				اكسر;
			}

			إذا (!box_visible(box))
				اكسر;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[يسار_2];
			data.redraw.y = y + box->padding[قمة_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_رسالة_REDRAW, &data);
		}
		اكسر;

	حالة CONTENT_رسالة_ERROR:
		hlcache_handle_release(object);

		o->content = عدم;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		اكسر;

	حالة CONTENT_رسالة_REDRAW:
		إذا (c->base.status != CONTENT_STATUS_LOADING) {
			اتحاد content_رسالة_data data = event->data;

			إذا (c->had_initial_layout == false) {
				اكسر;
			}

			إذا (!box_visible(box))
				اكسر;

			box_coords(box, &x, &y);

			إذا (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				صحيح width = box->padding[يسار_2] + box->width +
						box->padding[يمين_2];
				صحيح height = box->padding[قمة_2] + box->height +
						box->padding[BOTTOM];
				صحيح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				إذا (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} وإلا {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				إذا (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} وإلا {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				بدل (css_computed_background_repeat(
						box->style)) {
				حالة CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					اكسر;

				افتراضي:
					اكسر;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} وإلا {
				/* Non-background case */
				صحيح w = content_get_width(object);
				صحيح h = content_get_height(object);

				إذا (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				إذا (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[يسار_2];
				data.redraw.y += y + box->padding[قمة_2];
			}

			content_broadcast(&c->base, CONTENT_رسالة_REDRAW, &data);
		}
		اكسر;

	حالة CONTENT_رسالة_REFRESH:
		إذا (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		اكسر;

	حالة CONTENT_رسالة_LINK:
		/* Don't care about favicons that aren't on top level content */
		اكسر;

	حالة CONTENT_رسالة_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = عدم;
		اكسر;

	حالة CONTENT_رسالة_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		اكسر;

	حالة CONTENT_رسالة_SCROLL:
		إذا (box->scroll_x != عدم)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		إذا (box->scroll_y != عدم)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		اكسر;

	حالة CONTENT_رسالة_DRAGSAVE:
	{
		اتحاد content_رسالة_data رسالة_data;
		إذا (event->data.dragsave.content == عدم)
			رسالة_data.dragsave.content = object;
		وإلا
			رسالة_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_رسالة_DRAGSAVE, &رسالة_data);
	}
		اكسر;

	حالة CONTENT_رسالة_SAVELINK:
	حالة CONTENT_رسالة_POINTER:
	حالة CONTENT_رسالة_SELECTMENU:
	حالة CONTENT_رسالة_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		اكسر;

	حالة CONTENT_رسالة_CARET:
	{
		اتحاد html_focus_owner focus_owner;
		focus_owner.content = box;

		بدل (event->data.caret.type) {
		حالة CONTENT_CARET_REMOVE:
		حالة CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, عدم);
			اكسر;
		حالة CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			اكسر;
		}
	}
		اكسر;

	حالة CONTENT_رسالة_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اتحاد html_drag_owner drag_owner;
		drag_owner.content = box;

		بدل (event->data.drag.type) {
		حالة CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			اكسر;
		حالة CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			اكسر;
		حالة CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			اكسر;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		اكسر;

	حالة CONTENT_رسالة_SELECTION:
	{
		html_selection_type sel_type;
		اتحاد html_selection_owner sel_owner;

		إذا (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} وإلا {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		اكسر;

	افتراضي:
		اكسر;
	}

	إذا (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_رسالة_LOADING ||
	     event->type == CONTENT_رسالة_DONE ||
	     event->type == CONTENT_رسالة_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} وإلا إذا (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_رسالة_DONE &&
		   box != عدم &&
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
		إذا (ms_now > c->base.reformat_time) {
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

	إرجاع NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ساكن bool html_replace_object(بنية content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	أكد(object != عدم);
	أكد(object->box != عدم);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	إذا (object->content != عدم) {
		/* remove existing object */
		إذا (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = عدم;

		object->box->object = عدم;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), عدم,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	إذا (error != NSERROR_OK)
		إرجاع false;

	لكل (page = c; page != عدم; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	إرجاع true;
}

/**
 * schedule callback for object refresh
 */
ساكن فراغ html_object_refresh(فراغ *p)
{
	بنية content_html_object *object = p;
	nsurl *refresh_url;

	أكد(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	إذا (refresh_url == عدم)
		إرجاع;

	content_invalidate_reuse_data(object->content);

	إذا (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, بنية browser_window *bw)
{
	بنية content_html_object *object, *next;

	لكل (object = html->object_list; object != عدم; object = next) {
		next = object->next;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	إرجاع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	بنية content_html_object *object;

	لكل (object = htmlc->object_list;
	     object != عدم;
	     object = object->next) {
		إذا (object->content == عدم)
			استمر;

		بدل (content_get_status(object->content)) {
		حالة CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			اكسر;

		حالة CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			اكسر;

		افتراضي:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = عدم;
			إذا (object->box != عدم) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			اكسر;

		}
	}

	إرجاع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	بنية content_html_object *object, *next;

	لكل (object = html->object_list; object != عدم; object = next) {
		next = object->next;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	إرجاع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	بينما (html->object_list != عدم) {
		بنية content_html_object *victim = html->object_list;

		إذا (victim->content != عدم) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			إذا (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		حرر(victim);
	}
	إرجاع NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  بنية box *box,
		  content_type permitted_types,
		  bool background)
{
	بنية content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	إذا (c->aborted)
		إرجاع true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, حجم(بنية content_html_object));
	إذا (object == عدم) {
		إرجاع false;
	}

	إذا (box == عدم) {
		object_callback = html_object_nobox_callback;
	} وإلا {
		object_callback = html_object_callback;
	}

	object->parent = (بنية content *) c;
	object->next = عدم;
	object->content = عدم;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					عدم,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	إذا (error != NSERROR_OK) {
		حرر(object);
		إرجاع error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->رقم_objects++;
	إذا (box != عدم) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	إرجاع true;
}
