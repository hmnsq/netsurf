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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <nsutils/time.h>

#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "css/utils.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/interaction.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/object.h"

/* break reference loop */
સ્થિર શૂન્ય html_object_refresh(શૂન્ય *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
સંરચના content_html_object *html_get_objects(hlcache_handle *h, unsigned પૂર્ણાંક *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);
	પ્રતિજ્ઞા(n != ખાલી);

	*n = c->અંક_objects;

	પાછા c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

સ્થિર શૂન્ય
html_object_failed(સંરચના box *box, html_content *content, bool background)
{
	/* Nothing to do */
	પાછા;
}

/**
 * Update a box whose content has completed rendering.
 */

સ્થિર શૂન્ય
html_object_done(સંરચના box *box,
		 hlcache_handle *object,
		 bool background)
{
	સંરચના box *b;

	જો (background) {
		box->background = object;
		પાછા;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	પસંદગી (box->type) {
	સ્થિતિ BOX_TABLE:
		box->type = BOX_BLOCK;
		તોડો;
	મૂળભૂત:
		/* TODO: Any other box types need mapping? */
		તોડો;
	}

	જો (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		માટે (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		જ્યારે (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
સ્થિર nserror
html_object_nobox_callback(hlcache_handle *object,
			   અચલ hlcache_event *event,
			   શૂન્ય *pw)
{
	સંરચના content_html_object *chobject = pw;

	પસંદગી (event->type) {
	સ્થિતિ CONTENT_સંદેશ_ERROR:
		hlcache_handle_release(object);

		chobject->content = ખાલી;
		તોડો;

	મૂળભૂત:
		તોડો;
	}

	પાછા NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
સ્થિર nserror
html_object_callback(hlcache_handle *object,
		     અચલ hlcache_event *event,
		     શૂન્ય *pw)
{
	સંરચના content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	પૂર્ણાંક x, y;
	સંરચના box *box;

	box = o->box;

	પસંદગી (event->type) {
	સ્થિતિ CONTENT_સંદેશ_LOADING:
		જો (c->base.status != CONTENT_STATUS_LOADING && c->bw != ખાલી)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_READY:
		જો (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			જો (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		જો (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			સંઘ content_સંદેશ_data data;

			જો (c->had_initial_layout == false) {
				તોડો;
			}

			જો (!box_visible(box))
				તોડો;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[ડાબું_2];
			data.redraw.y = y + box->padding[ટોચ_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_સંદેશ_REDRAW, &data);
		}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		hlcache_handle_release(object);

		o->content = ખાલી;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		તોડો;

	સ્થિતિ CONTENT_સંદેશ_REDRAW:
		જો (c->base.status != CONTENT_STATUS_LOADING) {
			સંઘ content_સંદેશ_data data = event->data;

			જો (c->had_initial_layout == false) {
				તોડો;
			}

			જો (!box_visible(box))
				તોડો;

			box_coords(box, &x, &y);

			જો (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				પૂર્ણાંક width = box->padding[ડાબું_2] + box->width +
						box->padding[જમણું_2];
				પૂર્ણાંક height = box->padding[ટોચ_2] + box->height +
						box->padding[BOTTOM];
				પૂર્ણાંક t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				જો (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} નહીંતર {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				જો (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} નહીંતર {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				પસંદગી (css_computed_background_repeat(
						box->style)) {
				સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					તોડો;

				સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					તોડો;

				સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					તોડો;

				સ્થિતિ CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					તોડો;

				મૂળભૂત:
					તોડો;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} નહીંતર {
				/* Non-background case */
				પૂર્ણાંક w = content_get_width(object);
				પૂર્ણાંક h = content_get_height(object);

				જો (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				જો (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[ડાબું_2];
				data.redraw.y += y + box->padding[ટોચ_2];
			}

			content_broadcast(&c->base, CONTENT_સંદેશ_REDRAW, &data);
		}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_REFRESH:
		જો (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		તોડો;

	સ્થિતિ CONTENT_સંદેશ_LINK:
		/* Don't care about favicons that aren't on top level content */
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = ખાલી;
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_SCROLL:
		જો (box->scroll_x != ખાલી)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		જો (box->scroll_y != ખાલી)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_DRAGSAVE:
	{
		સંઘ content_સંદેશ_data સંદેશ_data;
		જો (event->data.dragsave.content == ખાલી)
			સંદેશ_data.dragsave.content = object;
		નહીંતર
			સંદેશ_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_સંદેશ_DRAGSAVE, &સંદેશ_data);
	}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_SAVELINK:
	સ્થિતિ CONTENT_સંદેશ_POINTER:
	સ્થિતિ CONTENT_સંદેશ_SELECTMENU:
	સ્થિતિ CONTENT_સંદેશ_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_CARET:
	{
		સંઘ html_focus_owner focus_owner;
		focus_owner.content = box;

		પસંદગી (event->data.caret.type) {
		સ્થિતિ CONTENT_CARET_REMOVE:
		સ્થિતિ CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ખાલી);
			તોડો;
		સ્થિતિ CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			તોડો;
		}
	}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		સંઘ html_drag_owner drag_owner;
		drag_owner.content = box;

		પસંદગી (event->data.drag.type) {
		સ્થિતિ CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			તોડો;
		સ્થિતિ CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			તોડો;
		સ્થિતિ CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			તોડો;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_SELECTION:
	{
		html_selection_type sel_type;
		સંઘ html_selection_owner sel_owner;

		જો (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} નહીંતર {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		તોડો;

	મૂળભૂત:
		તોડો;
	}

	જો (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_સંદેશ_LOADING ||
	     event->type == CONTENT_સંદેશ_DONE ||
	     event->type == CONTENT_સંદેશ_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} નહીંતર જો (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_સંદેશ_DONE &&
		   box != ખાલી &&
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
		જો (ms_now > c->base.reformat_time) {
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

	પાછા NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
સ્થિર bool html_replace_object(સંરચના content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	પ્રતિજ્ઞા(object != ખાલી);
	પ્રતિજ્ઞા(object->box != ખાલી);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	જો (object->content != ખાલી) {
		/* remove existing object */
		જો (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = ખાલી;

		object->box->object = ખાલી;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), ખાલી,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	જો (error != NSERROR_OK)
		પાછા false;

	માટે (page = c; page != ખાલી; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	પાછા true;
}

/**
 * schedule callback for object refresh
 */
સ્થિર શૂન્ય html_object_refresh(શૂન્ય *p)
{
	સંરચના content_html_object *object = p;
	nsurl *refresh_url;

	પ્રતિજ્ઞા(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	જો (refresh_url == ખાલી)
		પાછા;

	content_invalidate_reuse_data(object->content);

	જો (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, સંરચના browser_window *bw)
{
	સંરચના content_html_object *object, *next;

	માટે (object = html->object_list; object != ખાલી; object = next) {
		next = object->next;

		જો (object->content == ખાલી || object->box == ખાલી)
			ચાલુ;

		જો (content_get_type(object->content) == CONTENT_NONE)
			ચાલુ;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	પાછા NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	સંરચના content_html_object *object;

	માટે (object = htmlc->object_list;
	     object != ખાલી;
	     object = object->next) {
		જો (object->content == ખાલી)
			ચાલુ;

		પસંદગી (content_get_status(object->content)) {
		સ્થિતિ CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			તોડો;

		સ્થિતિ CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			તોડો;

		મૂળભૂત:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ખાલી;
			જો (object->box != ખાલી) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			તોડો;

		}
	}

	પાછા NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	સંરચના content_html_object *object, *next;

	માટે (object = html->object_list; object != ખાલી; object = next) {
		next = object->next;

		જો (object->content == ખાલી || object->box == ખાલી)
			ચાલુ;

		જો (content_get_type(object->content) == CONTENT_NONE)
			ચાલુ;

		જો (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	પાછા NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	જ્યારે (html->object_list != ખાલી) {
		સંરચના content_html_object *victim = html->object_list;

		જો (victim->content != ખાલી) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			જો (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		મુક્ત(victim);
	}
	પાછા NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  સંરચના box *box,
		  content_type permitted_types,
		  bool background)
{
	સંરચના content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	જો (c->aborted)
		પાછા true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, કદ(સંરચના content_html_object));
	જો (object == ખાલી) {
		પાછા false;
	}

	જો (box == ખાલી) {
		object_callback = html_object_nobox_callback;
	} નહીંતર {
		object_callback = html_object_callback;
	}

	object->parent = (સંરચના content *) c;
	object->next = ખાલી;
	object->content = ખાલી;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					ખાલી,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	જો (error != NSERROR_OK) {
		મુક્ત(object);
		પાછા error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->અંક_objects++;
	જો (box != ખાલી) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	પાછા true;
}
