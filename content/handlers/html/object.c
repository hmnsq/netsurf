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
কাঠামো content_html_object *html_get_objects(hlcache_handle *h, unsigned পূর্ণসংখ্যা *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != ফাঁকা);
	প্রতিজ্ঞা(n != ফাঁকা);

	*n = c->অঙ্ক_objects;

	ফেরত c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

স্থির শূন্য
html_object_failed(কাঠামো box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ফেরত;
}

/**
 * Update a box whose content has completed rendering.
 */

স্থির শূন্য
html_object_done(কাঠামো box *box,
		 hlcache_handle *object,
		 bool background)
{
	কাঠামো box *b;

	যদি (background) {
		box->background = object;
		ফেরত;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	নির্বাচন (box->type) {
	ক্ষেত্র BOX_TABLE:
		box->type = BOX_BLOCK;
		বিরতি;
	পূর্বনির্ধারিত:
		/* TODO: Any other box types need mapping? */
		বিরতি;
	}

	যদি (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		জন্য (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		যখন (box->পরবর্তী && (box->পরবর্তী->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->পরবর্তী = box->পরবর্তী->পরবর্তী;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
স্থির nserror
html_object_nobox_callback(hlcache_handle *object,
			   ধ্রুবক hlcache_event *event,
			   শূন্য *pw)
{
	কাঠামো content_html_object *chobject = pw;

	নির্বাচন (event->type) {
	ক্ষেত্র CONTENT_বার্তা_ERROR:
		hlcache_handle_release(object);

		chobject->content = ফাঁকা;
		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
	}

	ফেরত NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
স্থির nserror
html_object_callback(hlcache_handle *object,
		     ধ্রুবক hlcache_event *event,
		     শূন্য *pw)
{
	কাঠামো content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	পূর্ণসংখ্যা x, y;
	কাঠামো box *box;

	box = o->box;

	নির্বাচন (event->type) {
	ক্ষেত্র CONTENT_বার্তা_LOADING:
		যদি (c->base.status != CONTENT_STATUS_LOADING && c->bw != ফাঁকা)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_READY:
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
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		যদি (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			সংঘ content_বার্তা_তথ্য তথ্য;

			যদি (c->had_initial_layout == false) {
				বিরতি;
			}

			যদি (!box_visible(box))
				বিরতি;

			box_coords(box, &x, &y);

			তথ্য.redraw.x = x + box->padding[বাম_2];
			তথ্য.redraw.y = y + box->padding[শীর্ষ_2];
			তথ্য.redraw.width = box->width;
			তথ্য.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_বার্তা_REDRAW, &তথ্য);
		}
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_ERROR:
		hlcache_handle_release(object);

		o->content = ফাঁকা;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_REDRAW:
		যদি (c->base.status != CONTENT_STATUS_LOADING) {
			সংঘ content_বার্তা_তথ্য তথ্য = event->তথ্য;

			যদি (c->had_initial_layout == false) {
				বিরতি;
			}

			যদি (!box_visible(box))
				বিরতি;

			box_coords(box, &x, &y);

			যদি (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				পূর্ণসংখ্যা width = box->padding[বাম_2] + box->width +
						box->padding[ডান_2];
				পূর্ণসংখ্যা height = box->padding[শীর্ষ_2] + box->height +
						box->padding[BOTTOM];
				পূর্ণসংখ্যা t, h, l, w;

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
				নির্বাচন (css_computed_background_repeat(
						box->style)) {
				ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT:
					তথ্য.redraw.x = 0;
					তথ্য.redraw.y = 0;
					তথ্য.redraw.width = box->width;
					তথ্য.redraw.height = box->height;
					বিরতি;

				ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_X:
					তথ্য.redraw.x = 0;
					তথ্য.redraw.y += t;
					তথ্য.redraw.width = box->width;
					বিরতি;

				ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_Y:
					তথ্য.redraw.x += l;
					তথ্য.redraw.y = 0;
					তথ্য.redraw.height = box->height;
					বিরতি;

				ক্ষেত্র CSS_BACKGROUND_REPEAT_NO_REPEAT:
					তথ্য.redraw.x += l;
					তথ্য.redraw.y += t;
					বিরতি;

				পূর্বনির্ধারিত:
					বিরতি;
				}

				/* Add offset to box */
				তথ্য.redraw.x += x;
				তথ্য.redraw.y += y;

			} অন্যথা {
				/* Non-background case */
				পূর্ণসংখ্যা w = content_get_width(object);
				পূর্ণসংখ্যা h = content_get_height(object);

				যদি (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					তথ্য.redraw.x = তথ্য.redraw.x *
							box->width / w;
					তথ্য.redraw.width =
							তথ্য.redraw.width *
							box->width / w;
				}

				যদি (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					তথ্য.redraw.y = তথ্য.redraw.y *
							box->height / h;
					তথ্য.redraw.height =
							তথ্য.redraw.height *
							box->height / h;
				}

				তথ্য.redraw.x += x + box->padding[বাম_2];
				তথ্য.redraw.y += y + box->padding[শীর্ষ_2];
			}

			content_broadcast(&c->base, CONTENT_বার্তা_REDRAW, &তথ্য);
		}
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_REFRESH:
		যদি (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->তথ্য.delay * 1000,
					html_object_refresh, o);
		}

		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_LINK:
		/* Don't care about favicons that aren't on top level content */
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->তথ্য.jsthread) = ফাঁকা;
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_GETDIMS:
		*(event->তথ্য.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->তথ্য.getdims.viewport_height) =
				content__get_height(&c->base);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_SCROLL:
		যদি (box->scroll_x != ফাঁকা)
			scrollbar_set(box->scroll_x, event->তথ্য.scroll.x0,
					false);
		যদি (box->scroll_y != ফাঁকা)
			scrollbar_set(box->scroll_y, event->তথ্য.scroll.y0,
					false);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_DRAGSAVE:
	{
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
		যদি (event->তথ্য.dragsave.content == ফাঁকা)
			বার্তা_তথ্য.dragsave.content = object;
		অন্যথা
			বার্তা_তথ্য.dragsave.content =
					event->তথ্য.dragsave.content;

		content_broadcast(&c->base, CONTENT_বার্তা_DRAGSAVE, &বার্তা_তথ্য);
	}
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_SAVELINK:
	ক্ষেত্র CONTENT_বার্তা_POINTER:
	ক্ষেত্র CONTENT_বার্তা_SELECTMENU:
	ক্ষেত্র CONTENT_বার্তা_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->তথ্য);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_CARET:
	{
		সংঘ html_focus_owner focus_owner;
		focus_owner.content = box;

		নির্বাচন (event->তথ্য.caret.type) {
		ক্ষেত্র CONTENT_CARET_REMOVE:
		ক্ষেত্র CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ফাঁকা);
			বিরতি;
		ক্ষেত্র CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->তথ্য.caret.pos.x,
					event->তথ্য.caret.pos.y,
					event->তথ্য.caret.pos.height,
					event->তথ্য.caret.pos.clip);
			বিরতি;
		}
	}
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		সংঘ html_drag_owner drag_owner;
		drag_owner.content = box;

		নির্বাচন (event->তথ্য.drag.type) {
		ক্ষেত্র CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			বিরতি;
		ক্ষেত্র CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			বিরতি;
		ক্ষেত্র CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			বিরতি;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->তথ্য.drag.rect);
	}
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_SELECTION:
	{
		html_selection_type sel_type;
		সংঘ html_selection_owner sel_owner;

		যদি (event->তথ্য.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} অন্যথা {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->তথ্য.selection.read_only);
	}
		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
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
		   box != ফাঁকা &&
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

	ফেরত NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
স্থির bool html_replace_object(কাঠামো content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	প্রতিজ্ঞা(object != ফাঁকা);
	প্রতিজ্ঞা(object->box != ফাঁকা);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	যদি (object->content != ফাঁকা) {
		/* remove existing object */
		যদি (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = ফাঁকা;

		object->box->object = ফাঁকা;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), ফাঁকা,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	যদি (error != NSERROR_OK)
		ফেরত false;

	জন্য (page = c; page != ফাঁকা; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	ফেরত true;
}

/**
 * schedule callback for object refresh
 */
স্থির শূন্য html_object_refresh(শূন্য *p)
{
	কাঠামো content_html_object *object = p;
	nsurl *refresh_url;

	প্রতিজ্ঞা(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	যদি (refresh_url == ফাঁকা)
		ফেরত;

	content_invalidate_reuse_তথ্য(object->content);

	যদি (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, কাঠামো browser_window *bw)
{
	কাঠামো content_html_object *object, *পরবর্তী;

	জন্য (object = html->object_list; object != ফাঁকা; object = পরবর্তী) {
		পরবর্তী = object->পরবর্তী;

		যদি (object->content == ফাঁকা || object->box == ফাঁকা)
			চালু;

		যদি (content_get_type(object->content) == CONTENT_NONE)
			চালু;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	ফেরত NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	কাঠামো content_html_object *object;

	জন্য (object = htmlc->object_list;
	     object != ফাঁকা;
	     object = object->পরবর্তী) {
		যদি (object->content == ফাঁকা)
			চালু;

		নির্বাচন (content_get_status(object->content)) {
		ক্ষেত্র CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			বিরতি;

		ক্ষেত্র CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			বিরতি;

		পূর্বনির্ধারিত:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ফাঁকা;
			যদি (object->box != ফাঁকা) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			বিরতি;

		}
	}

	ফেরত NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	কাঠামো content_html_object *object, *পরবর্তী;

	জন্য (object = html->object_list; object != ফাঁকা; object = পরবর্তী) {
		পরবর্তী = object->পরবর্তী;

		যদি (object->content == ফাঁকা || object->box == ফাঁকা)
			চালু;

		যদি (content_get_type(object->content) == CONTENT_NONE)
			চালু;

		যদি (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ফেরত NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	যখন (html->object_list != ফাঁকা) {
		কাঠামো content_html_object *victim = html->object_list;

		যদি (victim->content != ফাঁকা) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			যদি (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->পরবর্তী;
		মুক্ত(victim);
	}
	ফেরত NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  কাঠামো box *box,
		  content_type permitted_types,
		  bool background)
{
	কাঠামো content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	যদি (c->aborted)
		ফেরত true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, আকার(কাঠামো content_html_object));
	যদি (object == ফাঁকা) {
		ফেরত false;
	}

	যদি (box == ফাঁকা) {
		object_callback = html_object_nobox_callback;
	} অন্যথা {
		object_callback = html_object_callback;
	}

	object->parent = (কাঠামো content *) c;
	object->পরবর্তী = ফাঁকা;
	object->content = ফাঁকা;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					ফাঁকা,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	যদি (error != NSERROR_OK) {
		মুক্ত(object);
		ফেরত error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->পরবর্তী = c->object_list;
	c->object_list = object;

	c->অঙ্ক_objects++;
	যদি (box != ফাঁকা) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ফেরত true;
}
