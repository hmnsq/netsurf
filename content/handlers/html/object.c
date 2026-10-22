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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <nsutils/time.h>

#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "css/utils.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/interaction.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/object.h"

/* break reference loop */
സ്ഥിതം ശൂന്യം html_object_refresh(ശൂന്യം *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ഘടന content_html_object *html_get_objects(hlcache_handle *h, unsigned പൂർണ്ണസംഖ്യ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);
	പ്രതിജ്ഞാ(n != ശൂന്യ);

	*n = c->അക്കം_objects;

	തിരികെ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

സ്ഥിതം ശൂന്യം
html_object_failed(ഘടന box *box, html_content *content, bool background)
{
	/* Nothing to do */
	തിരികെ;
}

/**
 * Update a box whose content has completed rendering.
 */

സ്ഥിതം ശൂന്യം
html_object_done(ഘടന box *box,
		 hlcache_handle *object,
		 bool background)
{
	ഘടന box *b;

	എങ്കിൽ (background) {
		box->background = object;
		തിരികെ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	തിരഞ്ഞെടുപ്പ് (box->type) {
	അവസ്ഥ BOX_TABLE:
		box->type = BOX_BLOCK;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* TODO: Any other box types need mapping? */
		നിർത്തുക;
	}

	എങ്കിൽ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		വേണ്ടി (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		ആകുമ്പോൾ (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
സ്ഥിതം nserror
html_object_nobox_callback(hlcache_handle *object,
			   സ്ഥിരം hlcache_event *event,
			   ശൂന്യം *pw)
{
	ഘടന content_html_object *chobject = pw;

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		hlcache_handle_release(object);

		chobject->content = ശൂന്യ;
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
സ്ഥിതം nserror
html_object_callback(hlcache_handle *object,
		     സ്ഥിരം hlcache_event *event,
		     ശൂന്യം *pw)
{
	ഘടന content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	പൂർണ്ണസംഖ്യ x, y;
	ഘടന box *box;

	box = o->box;

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ CONTENT_സന്ദേശം_LOADING:
		എങ്കിൽ (c->base.status != CONTENT_STATUS_LOADING && c->bw != ശൂന്യ)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_READY:
		എങ്കിൽ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			എങ്കിൽ (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		എങ്കിൽ (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			സംഘം content_സന്ദേശം_data data;

			എങ്കിൽ (c->had_initial_layout == false) {
				നിർത്തുക;
			}

			എങ്കിൽ (!box_visible(box))
				നിർത്തുക;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[ഇടത്_2];
			data.redraw.y = y + box->padding[മുകൾ_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_സന്ദേശം_REDRAW, &data);
		}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		hlcache_handle_release(object);

		o->content = ശൂന്യ;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_REDRAW:
		എങ്കിൽ (c->base.status != CONTENT_STATUS_LOADING) {
			സംഘം content_സന്ദേശം_data data = event->data;

			എങ്കിൽ (c->had_initial_layout == false) {
				നിർത്തുക;
			}

			എങ്കിൽ (!box_visible(box))
				നിർത്തുക;

			box_coords(box, &x, &y);

			എങ്കിൽ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				പൂർണ്ണസംഖ്യ width = box->padding[ഇടത്_2] + box->width +
						box->padding[വലത്_2];
				പൂർണ്ണസംഖ്യ height = box->padding[മുകൾ_2] + box->height +
						box->padding[BOTTOM];
				പൂർണ്ണസംഖ്യ t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				എങ്കിൽ (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} അല്ലെങ്കിൽ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				എങ്കിൽ (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} അല്ലെങ്കിൽ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				തിരഞ്ഞെടുപ്പ് (css_computed_background_repeat(
						box->style)) {
				അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					നിർത്തുക;

				അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					നിർത്തുക;

				അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					നിർത്തുക;

				അവസ്ഥ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					നിർത്തുക;

				സ്ഥിരസ്ഥിതി:
					നിർത്തുക;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} അല്ലെങ്കിൽ {
				/* Non-background case */
				പൂർണ്ണസംഖ്യ w = content_get_width(object);
				പൂർണ്ണസംഖ്യ h = content_get_height(object);

				എങ്കിൽ (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				എങ്കിൽ (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[ഇടത്_2];
				data.redraw.y += y + box->padding[മുകൾ_2];
			}

			content_broadcast(&c->base, CONTENT_സന്ദേശം_REDRAW, &data);
		}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_REFRESH:
		എങ്കിൽ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_LINK:
		/* Don't care about favicons that aren't on top level content */
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = ശൂന്യ;
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_SCROLL:
		എങ്കിൽ (box->scroll_x != ശൂന്യ)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		എങ്കിൽ (box->scroll_y != ശൂന്യ)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_DRAGSAVE:
	{
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;
		എങ്കിൽ (event->data.dragsave.content == ശൂന്യ)
			സന്ദേശം_data.dragsave.content = object;
		അല്ലെങ്കിൽ
			സന്ദേശം_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_സന്ദേശം_DRAGSAVE, &സന്ദേശം_data);
	}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_SAVELINK:
	അവസ്ഥ CONTENT_സന്ദേശം_POINTER:
	അവസ്ഥ CONTENT_സന്ദേശം_SELECTMENU:
	അവസ്ഥ CONTENT_സന്ദേശം_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_CARET:
	{
		സംഘം html_focus_owner focus_owner;
		focus_owner.content = box;

		തിരഞ്ഞെടുപ്പ് (event->data.caret.type) {
		അവസ്ഥ CONTENT_CARET_REMOVE:
		അവസ്ഥ CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ശൂന്യ);
			നിർത്തുക;
		അവസ്ഥ CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			നിർത്തുക;
		}
	}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		സംഘം html_drag_owner drag_owner;
		drag_owner.content = box;

		തിരഞ്ഞെടുപ്പ് (event->data.drag.type) {
		അവസ്ഥ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			നിർത്തുക;
		അവസ്ഥ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			നിർത്തുക;
		അവസ്ഥ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			നിർത്തുക;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_SELECTION:
	{
		html_selection_type sel_type;
		സംഘം html_selection_owner sel_owner;

		എങ്കിൽ (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} അല്ലെങ്കിൽ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	എങ്കിൽ (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_സന്ദേശം_LOADING ||
	     event->type == CONTENT_സന്ദേശം_DONE ||
	     event->type == CONTENT_സന്ദേശം_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} അല്ലെങ്കിൽ എങ്കിൽ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_സന്ദേശം_DONE &&
		   box != ശൂന്യ &&
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
		എങ്കിൽ (ms_now > c->base.reformat_time) {
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

	തിരികെ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
സ്ഥിതം bool html_replace_object(ഘടന content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	പ്രതിജ്ഞാ(object != ശൂന്യ);
	പ്രതിജ്ഞാ(object->box != ശൂന്യ);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	എങ്കിൽ (object->content != ശൂന്യ) {
		/* remove existing object */
		എങ്കിൽ (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = ശൂന്യ;

		object->box->object = ശൂന്യ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), ശൂന്യ,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ false;

	വേണ്ടി (page = c; page != ശൂന്യ; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	തിരികെ true;
}

/**
 * schedule callback for object refresh
 */
സ്ഥിതം ശൂന്യം html_object_refresh(ശൂന്യം *p)
{
	ഘടന content_html_object *object = p;
	nsurl *refresh_url;

	പ്രതിജ്ഞാ(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	എങ്കിൽ (refresh_url == ശൂന്യ)
		തിരികെ;

	content_invalidate_reuse_data(object->content);

	എങ്കിൽ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ഘടന browser_window *bw)
{
	ഘടന content_html_object *object, *next;

	വേണ്ടി (object = html->object_list; object != ശൂന്യ; object = next) {
		next = object->next;

		എങ്കിൽ (object->content == ശൂന്യ || object->box == ശൂന്യ)
			തുടരുക;

		എങ്കിൽ (content_get_type(object->content) == CONTENT_NONE)
			തുടരുക;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ഘടന content_html_object *object;

	വേണ്ടി (object = htmlc->object_list;
	     object != ശൂന്യ;
	     object = object->next) {
		എങ്കിൽ (object->content == ശൂന്യ)
			തുടരുക;

		തിരഞ്ഞെടുപ്പ് (content_get_status(object->content)) {
		അവസ്ഥ CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			നിർത്തുക;

		അവസ്ഥ CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ശൂന്യ;
			എങ്കിൽ (object->box != ശൂന്യ) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			നിർത്തുക;

		}
	}

	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ഘടന content_html_object *object, *next;

	വേണ്ടി (object = html->object_list; object != ശൂന്യ; object = next) {
		next = object->next;

		എങ്കിൽ (object->content == ശൂന്യ || object->box == ശൂന്യ)
			തുടരുക;

		എങ്കിൽ (content_get_type(object->content) == CONTENT_NONE)
			തുടരുക;

		എങ്കിൽ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	ആകുമ്പോൾ (html->object_list != ശൂന്യ) {
		ഘടന content_html_object *victim = html->object_list;

		എങ്കിൽ (victim->content != ശൂന്യ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			എങ്കിൽ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		സ്വതന്ത്രം(victim);
	}
	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ഘടന box *box,
		  content_type permitted_types,
		  bool background)
{
	ഘടന content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	എങ്കിൽ (c->aborted)
		തിരികെ true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, വലുപ്പം(ഘടന content_html_object));
	എങ്കിൽ (object == ശൂന്യ) {
		തിരികെ false;
	}

	എങ്കിൽ (box == ശൂന്യ) {
		object_callback = html_object_nobox_callback;
	} അല്ലെങ്കിൽ {
		object_callback = html_object_callback;
	}

	object->parent = (ഘടന content *) c;
	object->next = ശൂന്യ;
	object->content = ശൂന്യ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					ശൂന്യ,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(object);
		തിരികെ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->അക്കം_objects++;
	എങ്കിൽ (box != ശൂന്യ) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	തിരികെ true;
}
