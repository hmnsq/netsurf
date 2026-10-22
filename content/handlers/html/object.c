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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <nsutils/time.h>

#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "css/utils.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/interaction.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/object.h"

/* break reference loop */
स्थिर शून्य html_object_refresh(शून्य *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
संरचना content_html_object *html_get_objects(hlcache_handle *h, अचिन्हित पूर्णाङ्क *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->अङ्क_objects;

	फर्काऊ c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

स्थिर शून्य
html_object_failed(संरचना box *box, html_content *content, bool background)
{
	/* Nothing to do */
	फर्काऊ;
}

/**
 * Update a box whose content has completed rendering.
 */

स्थिर शून्य
html_object_done(संरचना box *box,
		 hlcache_handle *object,
		 bool background)
{
	संरचना box *b;

	यदि (background) {
		box->background = object;
		फर्काऊ;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	विकल्प (box->type) {
	पक्ष BOX_TABLE:
		box->type = BOX_BLOCK;
		रोक;
	स्वभाव:
		/* TODO: Any other box types need mapping? */
		रोक;
	}

	यदि (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		लागि (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		जबसम्म (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
स्थिर nserror
html_object_nobox_callback(hlcache_handle *object,
			   अचर hlcache_event *event,
			   शून्य *pw)
{
	संरचना content_html_object *chobject = pw;

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देश_ERROR:
		hlcache_handle_release(object);

		chobject->content = रिक्त;
		रोक;

	स्वभाव:
		रोक;
	}

	फर्काऊ NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
स्थिर nserror
html_object_callback(hlcache_handle *object,
		     अचर hlcache_event *event,
		     शून्य *pw)
{
	संरचना content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	पूर्णाङ्क x, y;
	संरचना box *box;

	box = o->box;

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देश_LOADING:
		यदि (c->base.status != CONTENT_STATUS_LOADING && c->bw != रिक्त)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		रोक;

	पक्ष CONTENT_सन्देश_READY:
		यदि (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			यदि (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		रोक;

	पक्ष CONTENT_सन्देश_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		यदि (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			सङ्घ content_सन्देश_data data;

			यदि (c->had_initial_layout == false) {
				रोक;
			}

			यदि (!box_visible(box))
				रोक;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[बायाँ_2];
			data.redraw.y = y + box->padding[शीर्ष_2];
			data.redraw.width = box->width;
			data.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_सन्देश_REDRAW, &data);
		}
		रोक;

	पक्ष CONTENT_सन्देश_ERROR:
		hlcache_handle_release(object);

		o->content = रिक्त;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		रोक;

	पक्ष CONTENT_सन्देश_REDRAW:
		यदि (c->base.status != CONTENT_STATUS_LOADING) {
			सङ्घ content_सन्देश_data data = event->data;

			यदि (c->had_initial_layout == false) {
				रोक;
			}

			यदि (!box_visible(box))
				रोक;

			box_coords(box, &x, &y);

			यदि (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				पूर्णाङ्क width = box->padding[बायाँ_2] + box->width +
						box->padding[दायाँ_2];
				पूर्णाङ्क height = box->padding[शीर्ष_2] + box->height +
						box->padding[BOTTOM];
				पूर्णाङ्क t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				यदि (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} अन्यथा {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				यदि (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} अन्यथा {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				विकल्प (css_computed_background_repeat(
						box->style)) {
				पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
					data.redraw.x = 0;
					data.redraw.y = 0;
					data.redraw.width = box->width;
					data.redraw.height = box->height;
					रोक;

				पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
					data.redraw.x = 0;
					data.redraw.y += t;
					data.redraw.width = box->width;
					रोक;

				पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
					data.redraw.x += l;
					data.redraw.y = 0;
					data.redraw.height = box->height;
					रोक;

				पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
					data.redraw.x += l;
					data.redraw.y += t;
					रोक;

				स्वभाव:
					रोक;
				}

				/* Add offset to box */
				data.redraw.x += x;
				data.redraw.y += y;

			} अन्यथा {
				/* Non-background case */
				पूर्णाङ्क w = content_get_width(object);
				पूर्णाङ्क h = content_get_height(object);

				यदि (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					data.redraw.x = data.redraw.x *
							box->width / w;
					data.redraw.width =
							data.redraw.width *
							box->width / w;
				}

				यदि (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					data.redraw.y = data.redraw.y *
							box->height / h;
					data.redraw.height =
							data.redraw.height *
							box->height / h;
				}

				data.redraw.x += x + box->padding[बायाँ_2];
				data.redraw.y += y + box->padding[शीर्ष_2];
			}

			content_broadcast(&c->base, CONTENT_सन्देश_REDRAW, &data);
		}
		रोक;

	पक्ष CONTENT_सन्देश_REFRESH:
		यदि (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->data.delay * 1000,
					html_object_refresh, o);
		}

		रोक;

	पक्ष CONTENT_सन्देश_LINK:
		/* Don't care about favicons that aren't on top level content */
		रोक;

	पक्ष CONTENT_सन्देश_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->data.jsthread) = रिक्त;
		रोक;

	पक्ष CONTENT_सन्देश_GETDIMS:
		*(event->data.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->data.getdims.viewport_height) =
				content__get_height(&c->base);
		रोक;

	पक्ष CONTENT_सन्देश_SCROLL:
		यदि (box->scroll_x != रिक्त)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		यदि (box->scroll_y != रिक्त)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		रोक;

	पक्ष CONTENT_सन्देश_DRAGSAVE:
	{
		सङ्घ content_सन्देश_data सन्देश_data;
		यदि (event->data.dragsave.content == रिक्त)
			सन्देश_data.dragsave.content = object;
		अन्यथा
			सन्देश_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_सन्देश_DRAGSAVE, &सन्देश_data);
	}
		रोक;

	पक्ष CONTENT_सन्देश_SAVELINK:
	पक्ष CONTENT_सन्देश_POINTER:
	पक्ष CONTENT_सन्देश_SELECTMENU:
	पक्ष CONTENT_सन्देश_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->data);
		रोक;

	पक्ष CONTENT_सन्देश_CARET:
	{
		सङ्घ html_focus_owner focus_owner;
		focus_owner.content = box;

		विकल्प (event->data.caret.type) {
		पक्ष CONTENT_CARET_REMOVE:
		पक्ष CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, रिक्त);
			रोक;
		पक्ष CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->data.caret.pos.x,
					event->data.caret.pos.y,
					event->data.caret.pos.height,
					event->data.caret.pos.clip);
			रोक;
		}
	}
		रोक;

	पक्ष CONTENT_सन्देश_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		सङ्घ html_drag_owner drag_owner;
		drag_owner.content = box;

		विकल्प (event->data.drag.type) {
		पक्ष CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			रोक;
		पक्ष CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			रोक;
		पक्ष CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			रोक;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->data.drag.rect);
	}
		रोक;

	पक्ष CONTENT_सन्देश_SELECTION:
	{
		html_selection_type sel_type;
		सङ्घ html_selection_owner sel_owner;

		यदि (event->data.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} अन्यथा {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->data.selection.read_only);
	}
		रोक;

	स्वभाव:
		रोक;
	}

	यदि (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_सन्देश_LOADING ||
	     event->type == CONTENT_सन्देश_DONE ||
	     event->type == CONTENT_सन्देश_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} अन्यथा यदि (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_सन्देश_DONE &&
		   box != रिक्त &&
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
		यदि (ms_now > c->base.reformat_time) {
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

	फर्काऊ NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool html_replace_object(संरचना content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	प्रतिज्ञा(object != रिक्त);
	प्रतिज्ञा(object->box != रिक्त);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	यदि (object->content != रिक्त) {
		/* remove existing object */
		यदि (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = रिक्त;

		object->box->object = रिक्त;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), रिक्त,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	यदि (error != NSERROR_OK)
		फर्काऊ false;

	लागि (page = c; page != रिक्त; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	फर्काऊ true;
}

/**
 * schedule callback for object refresh
 */
स्थिर शून्य html_object_refresh(शून्य *p)
{
	संरचना content_html_object *object = p;
	nsurl *refresh_url;

	प्रतिज्ञा(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	यदि (refresh_url == रिक्त)
		फर्काऊ;

	content_invalidate_reuse_data(object->content);

	यदि (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, संरचना browser_window *bw)
{
	संरचना content_html_object *object, *next;

	लागि (object = html->object_list; object != रिक्त; object = next) {
		next = object->next;

		यदि (object->content == रिक्त || object->box == रिक्त)
			जारी;

		यदि (content_get_type(object->content) == CONTENT_NONE)
			जारी;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	फर्काऊ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	संरचना content_html_object *object;

	लागि (object = htmlc->object_list;
	     object != रिक्त;
	     object = object->next) {
		यदि (object->content == रिक्त)
			जारी;

		विकल्प (content_get_status(object->content)) {
		पक्ष CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			रोक;

		पक्ष CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			रोक;

		स्वभाव:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = रिक्त;
			यदि (object->box != रिक्त) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			रोक;

		}
	}

	फर्काऊ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	संरचना content_html_object *object, *next;

	लागि (object = html->object_list; object != रिक्त; object = next) {
		next = object->next;

		यदि (object->content == रिक्त || object->box == रिक्त)
			जारी;

		यदि (content_get_type(object->content) == CONTENT_NONE)
			जारी;

		यदि (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	फर्काऊ NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	जबसम्म (html->object_list != रिक्त) {
		संरचना content_html_object *victim = html->object_list;

		यदि (victim->content != रिक्त) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			यदि (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		मुक्त(victim);
	}
	फर्काऊ NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  संरचना box *box,
		  content_type permitted_types,
		  bool background)
{
	संरचना content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	यदि (c->aborted)
		फर्काऊ true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = सुस्मृति(1, परिमाण(संरचना content_html_object));
	यदि (object == रिक्त) {
		फर्काऊ false;
	}

	यदि (box == रिक्त) {
		object_callback = html_object_nobox_callback;
	} अन्यथा {
		object_callback = html_object_callback;
	}

	object->parent = (संरचना content *) c;
	object->next = रिक्त;
	object->content = रिक्त;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					रिक्त,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	यदि (error != NSERROR_OK) {
		मुक्त(object);
		फर्काऊ error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->अङ्क_objects++;
	यदि (box != रिक्त) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	फर्काऊ true;
}
