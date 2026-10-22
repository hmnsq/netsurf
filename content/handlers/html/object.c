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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>

#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/interaction.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/object.h"

/* break reference loop */
ସ୍ଥିର ଶୂନ୍ୟ html_object_refresh(ଶୂନ୍ୟ *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ସଂରଚନା content_html_object *html_get_objects(hlcache_handle *h, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(n != ଖାଲି);

	*n = c->ଅଙ୍କ_objects;

	ଫେରନ୍ତୁ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ସ୍ଥିର ଶୂନ୍ୟ
html_object_failed(ସଂରଚନା box *box, html_content *content, bool background)
{
	/* Nothing to do */
	ଫେରନ୍ତୁ;
}

/**
 * Update a box whose content has completed rendering.
 */

ସ୍ଥିର ଶୂନ୍ୟ
html_object_done(ସଂରଚନା box *box,
		 hlcache_handle *object,
		 bool background)
{
	ସଂରଚନା box *b;

	ଯଦି (background) {
		box->background = object;
		ଫେରନ୍ତୁ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	ବାଛ (box->type) {
	ସ୍ଥିତି BOX_TABLE:
		box->type = BOX_BLOCK;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* TODO: Any other box types need mapping? */
		ଭାଙ୍ଗ;
	}

	ଯଦି (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		ପାଇଁ (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		ଯେତେବେଳେ (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ସ୍ଥିର nserror
html_object_nobox_callback(hlcache_handle *object,
			   ଅଚଳ hlcache_event *event,
			   ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content_html_object *chobject = pw;

	ବାଛ (event->type) {
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		hlcache_handle_release(object);

		chobject->content = ଖାଲି;
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ସ୍ଥିର nserror
html_object_callback(hlcache_handle *object,
		     ଅଚଳ hlcache_event *event,
		     ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;
	ସଂରଚନା box *box;

	box = o->box;

	ବାଛ (event->type) {
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_LOADING:
		ଯଦି (c->base.status != CONTENT_STATUS_LOADING && c->bw != ଖାଲି)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_READY:
		ଯଦି (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			ଯଦି (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		ଯଦି (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			ସଂଘ content_ସନ୍ଦେଶ_data data;

			ଯଦି (c->had_initial_layout == false) {
				ଭାଙ୍ଗ;
			}

			ଯଦି (!box_visible(box))
				ଭାଙ୍ଗ;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[ବାମ_2];
			data.redraw.y = y + box->padding[ଶୀର୍ଷ_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_ସନ୍ଦେଶ_REDRAW, &data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		hlcache_handle_release(object);

		o->content = ଖାଲି;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_REDRAW:
		ଯଦି (c->base.status != CONTENT_STATUS_LOADING) {
			ସଂଘ content_ସନ୍ଦେଶ_data data = event->data;

			ଯଦି (c->had_initial_layout == false) {
				ଭାଙ୍ଗ;
			}

			ଯଦି (!box_visible(box))
				ଭାଙ୍ଗ;

			box_coords(box, &x, &y);

			ଯଦି (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = box->padding[ବାମ_2] + box->width +
						box->padding[ଡାହାଣ_2];
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା height = box->padding[ଶୀର୍ଷ_2] + box->height +
						box->padding[BOTTOM];
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				ଯଦି (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} ଅନ୍ୟଥା {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				ଯଦି (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} ଅନ୍ୟଥା {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				ବାଛ (css_computed_background_repeat(
						box->style)) {
				ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					ଭାଙ୍ଗ;

				ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					ଭାଙ୍ଗ;

				ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					ଭାଙ୍ଗ;

				ସ୍ଥିତି CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					ଭାଙ୍ଗ;

				ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
					ଭାଙ୍ଗ;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} ଅନ୍ୟଥା {
				/* Non-background case */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା w = content_get_width(object);
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = content_get_height(object);

				ଯଦି (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				ଯଦି (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[ବାମ_2];
				data.redraw.y += y + box->padding[ଶୀର୍ଷ_2];
			}

			content_broadcast(&c->base, CONTENT_ସନ୍ଦେଶ_REDRAW, &data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_REFRESH:
		ଯଦି (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_LINK:
		/* Don't care about favicons that aren't on top level content */
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = ଖାଲି;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_SCROLL:
		ଯଦି (box->scroll_x != ଖାଲି)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		ଯଦି (box->scroll_y != ଖାଲି)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DRAGSAVE:
	{
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
		ଯଦି (event->data.dragsave.content == ଖାଲି)
			ସନ୍ଦେଶ_data.dragsave.content = object;
		ଅନ୍ୟଥା
			ସନ୍ଦେଶ_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_ସନ୍ଦେଶ_DRAGSAVE, &ସନ୍ଦେଶ_data);
	}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_SAVELINK:
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_POINTER:
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_SELECTMENU:
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_CARET:
	{
		ସଂଘ html_focus_owner focus_owner;
		focus_owner.content = box;

		ବାଛ (event->data.caret.type) {
		ସ୍ଥିତି CONTENT_CARET_REMOVE:
		ସ୍ଥିତି CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ଖାଲି);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			ଭାଙ୍ଗ;
		}
	}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		ସଂଘ html_drag_owner drag_owner;
		drag_owner.content = box;

		ବାଛ (event->data.drag.type) {
		ସ୍ଥିତି CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ଭାଙ୍ଗ;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_SELECTION:
	{
		html_selection_type sel_type;
		ସଂଘ html_selection_owner sel_owner;

		ଯଦି (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ଅନ୍ୟଥା {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଯଦି (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_ସନ୍ଦେଶ_LOADING ||
	     event->type == CONTENT_ସନ୍ଦେଶ_DONE ||
	     event->type == CONTENT_ସନ୍ଦେଶ_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} ଅନ୍ୟଥା ଯଦି (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_ସନ୍ଦେଶ_DONE &&
		   box != ଖାଲି &&
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
		ଯଦି (ms_now > c->base.reformat_time) {
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

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ସ୍ଥିର bool html_replace_object(ସଂରଚନା content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	ପ୍ରତିଜ୍ଞା(object != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(object->box != ଖାଲି);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ଯଦି (object->content != ଖାଲି) {
		/* remove existing object */
		ଯଦି (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = ଖାଲି;

		object->box->object = ଖାଲି;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), ଖାଲି,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (page = c; page != ଖାଲି; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	ଫେରନ୍ତୁ true;
}

/**
 * schedule callback for object refresh
 */
ସ୍ଥିର ଶୂନ୍ୟ html_object_refresh(ଶୂନ୍ୟ *p)
{
	ସଂରଚନା content_html_object *object = p;
	nsurl *refresh_url;

	ପ୍ରତିଜ୍ଞା(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ଯଦି (refresh_url == ଖାଲି)
		ଫେରନ୍ତୁ;

	content_invalidate_reuse_data(object->content);

	ଯଦି (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ସଂରଚନା browser_window *bw)
{
	ସଂରଚନା content_html_object *object, *next;

	ପାଇଁ (object = html->object_list; object != ଖାଲି; object = next) {
		next = object->next;

		ଯଦି (object->content == ଖାଲି || object->box == ଖାଲି)
			ଜାରି;

		ଯଦି (content_get_type(object->content) == CONTENT_NONE)
			ଜାରି;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ସଂରଚନା content_html_object *object;

	ପାଇଁ (object = htmlc->object_list;
	     object != ଖାଲି;
	     object = object->next) {
		ଯଦି (object->content == ଖାଲି)
			ଜାରି;

		ବାଛ (content_get_status(object->content)) {
		ସ୍ଥିତି CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ଖାଲି;
			ଯଦି (object->box != ଖାଲି) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			ଭାଙ୍ଗ;

		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ସଂରଚନା content_html_object *object, *next;

	ପାଇଁ (object = html->object_list; object != ଖାଲି; object = next) {
		next = object->next;

		ଯଦି (object->content == ଖାଲି || object->box == ଖାଲି)
			ଜାରି;

		ଯଦି (content_get_type(object->content) == CONTENT_NONE)
			ଜାରି;

		ଯଦି (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ଯେତେବେଳେ (html->object_list != ଖାଲି) {
		ସଂରଚନା content_html_object *victim = html->object_list;

		ଯଦି (victim->content != ଖାଲି) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ଯଦି (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		ମୁକ୍ତ(victim);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ସଂରଚନା box *box,
		  content_type permitted_types,
		  bool background)
{
	ସଂରଚନା content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ଯଦି (c->aborted)
		ଫେରନ୍ତୁ true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, ଆକାର(ସଂରଚନା content_html_object));
	ଯଦି (object == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (box == ଖାଲି) {
		object_callback = html_object_nobox_callback;
	} ଅନ୍ୟଥା {
		object_callback = html_object_callback;
	}

	object->parent = (ସଂରଚନା content *) c;
	object->next = ଖାଲି;
	object->content = ଖାଲି;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					ଖାଲି,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(object);
		ଫେରନ୍ତୁ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->ଅଙ୍କ_objects++;
	ଯଦି (box != ଖାଲି) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ଫେରନ୍ତୁ true;
}
