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

#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <stdlib.h>
#समावेश <nsutils/time.h>

#समावेश "utils/corestrings.h"
#समावेश "utils/config.h"
#समावेश "utils/log.h"
#समावेश "utils/nsoption.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/misc.h"
#समावेश "content/hlcache.h"
#समावेश "css/utils.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/interaction.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/object.h"

/* break reference loop */
अटल व्योम html_object_refresh(व्योम *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
काष्ठा content_html_object *html_get_objects(hlcache_handle *h, अचिन्हित पूर्णांक *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);
	निश्चित(n != शून्य);

	*n = c->अंक_objects;

	वापस c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

अटल व्योम
html_object_failed(काष्ठा box *box, html_content *content, bool background)
{
	/* Nothing to do */
	वापस;
}

/**
 * Update a box whose content has completed rendering.
 */

अटल व्योम
html_object_done(काष्ठा box *box,
		 hlcache_handle *object,
		 bool background)
{
	काष्ठा box *b;

	अगर (background) {
		box->background = object;
		वापस;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	चयन (box->type) {
	हाल BOX_TABLE:
		box->type = BOX_BLOCK;
		अवरोध;
	शेष:
		/* TODO: Any other box types need mapping? */
		अवरोध;
	}

	अगर (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		क्रम (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		जबतक (box->अग्र && (box->अग्र->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->अग्र = box->अग्र->अग्र;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
अटल nserror
html_object_nobox_callback(hlcache_handle *object,
			   स्थिर hlcache_event *event,
			   व्योम *pw)
{
	काष्ठा content_html_object *chobject = pw;

	चयन (event->type) {
	हाल CONTENT_संदेश_ERROR:
		hlcache_handle_release(object);

		chobject->content = शून्य;
		अवरोध;

	शेष:
		अवरोध;
	}

	वापस NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
अटल nserror
html_object_callback(hlcache_handle *object,
		     स्थिर hlcache_event *event,
		     व्योम *pw)
{
	काष्ठा content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	पूर्णांक x, y;
	काष्ठा box *box;

	box = o->box;

	चयन (event->type) {
	हाल CONTENT_संदेश_LOADING:
		अगर (c->base.status != CONTENT_STATUS_LOADING && c->bw != शून्य)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		अवरोध;

	हाल CONTENT_संदेश_READY:
		अगर (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			अगर (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.available_height);
		}
		अवरोध;

	हाल CONTENT_संदेश_DONE:
		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_done(box, object, o->background);

		अगर (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			जोड़ content_संदेश_तथ्य तथ्य;

			अगर (c->had_initial_layout == false) {
				अवरोध;
			}

			अगर (!box_visible(box))
				अवरोध;

			box_coords(box, &x, &y);

			तथ्य.redraw.x = x + box->padding[बायाँ_2];
			तथ्य.redraw.y = y + box->padding[शीर्ष_2];
			तथ्य.redraw.width = box->width;
			तथ्य.redraw.height = box->height;

			content_broadcast(&c->base, CONTENT_संदेश_REDRAW, &तथ्य);
		}
		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		hlcache_handle_release(object);

		o->content = शून्य;

		c->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		html_object_failed(box, c, o->background);

		अवरोध;

	हाल CONTENT_संदेश_REDRAW:
		अगर (c->base.status != CONTENT_STATUS_LOADING) {
			जोड़ content_संदेश_तथ्य तथ्य = event->तथ्य;

			अगर (c->had_initial_layout == false) {
				अवरोध;
			}

			अगर (!box_visible(box))
				अवरोध;

			box_coords(box, &x, &y);

			अगर (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				पूर्णांक width = box->padding[बायाँ_2] + box->width +
						box->padding[दायाँ_2];
				पूर्णांक height = box->padding[शीर्ष_2] + box->height +
						box->padding[BOTTOM];
				पूर्णांक t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_width(box->background);
				अगर (hunit == CSS_UNIT_PCT) {
					l = (width - w) * hpos / INTTOFIX(100);
				} अन्यथा {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_height(box->background);
				अगर (vunit == CSS_UNIT_PCT) {
					t = (height - h) * vpos / INTTOFIX(100);
				} अन्यथा {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				चयन (css_computed_background_repeat(
						box->style)) {
				हाल CSS_BACKGROUND_REPEAT_REPEAT:
					तथ्य.redraw.x = 0;
					तथ्य.redraw.y = 0;
					तथ्य.redraw.width = box->width;
					तथ्य.redraw.height = box->height;
					अवरोध;

				हाल CSS_BACKGROUND_REPEAT_REPEAT_X:
					तथ्य.redraw.x = 0;
					तथ्य.redraw.y += t;
					तथ्य.redraw.width = box->width;
					अवरोध;

				हाल CSS_BACKGROUND_REPEAT_REPEAT_Y:
					तथ्य.redraw.x += l;
					तथ्य.redraw.y = 0;
					तथ्य.redraw.height = box->height;
					अवरोध;

				हाल CSS_BACKGROUND_REPEAT_NO_REPEAT:
					तथ्य.redraw.x += l;
					तथ्य.redraw.y += t;
					अवरोध;

				शेष:
					अवरोध;
				}

				/* Add offset to box */
				तथ्य.redraw.x += x;
				तथ्य.redraw.y += y;

			} अन्यथा {
				/* Non-background case */
				पूर्णांक w = content_get_width(object);
				पूर्णांक h = content_get_height(object);

				अगर (w != 0 && box->width != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					तथ्य.redraw.x = तथ्य.redraw.x *
							box->width / w;
					तथ्य.redraw.width =
							तथ्य.redraw.width *
							box->width / w;
				}

				अगर (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					तथ्य.redraw.y = तथ्य.redraw.y *
							box->height / h;
					तथ्य.redraw.height =
							तथ्य.redraw.height *
							box->height / h;
				}

				तथ्य.redraw.x += x + box->padding[बायाँ_2];
				तथ्य.redraw.y += y + box->padding[शीर्ष_2];
			}

			content_broadcast(&c->base, CONTENT_संदेश_REDRAW, &तथ्य);
		}
		अवरोध;

	हाल CONTENT_संदेश_REFRESH:
		अगर (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->schedule(event->तथ्य.delay * 1000,
					html_object_refresh, o);
		}

		अवरोध;

	हाल CONTENT_संदेश_LINK:
		/* Don't care about favicons that aren't on top level content */
		अवरोध;

	हाल CONTENT_संदेश_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->तथ्य.jsthread) = शून्य;
		अवरोध;

	हाल CONTENT_संदेश_GETDIMS:
		*(event->तथ्य.getdims.viewport_width) =
				content__get_width(&c->base);
		*(event->तथ्य.getdims.viewport_height) =
				content__get_height(&c->base);
		अवरोध;

	हाल CONTENT_संदेश_SCROLL:
		अगर (box->scroll_x != शून्य)
			scrollbar_set(box->scroll_x, event->तथ्य.scroll.x0,
					false);
		अगर (box->scroll_y != शून्य)
			scrollbar_set(box->scroll_y, event->तथ्य.scroll.y0,
					false);
		अवरोध;

	हाल CONTENT_संदेश_DRAGSAVE:
	{
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;
		अगर (event->तथ्य.dragsave.content == शून्य)
			संदेश_तथ्य.dragsave.content = object;
		अन्यथा
			संदेश_तथ्य.dragsave.content =
					event->तथ्य.dragsave.content;

		content_broadcast(&c->base, CONTENT_संदेश_DRAGSAVE, &संदेश_तथ्य);
	}
		अवरोध;

	हाल CONTENT_संदेश_SAVELINK:
	हाल CONTENT_संदेश_POINTER:
	हाल CONTENT_संदेश_SELECTMENU:
	हाल CONTENT_संदेश_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, &event->तथ्य);
		अवरोध;

	हाल CONTENT_संदेश_CARET:
	{
		जोड़ html_focus_owner focus_owner;
		focus_owner.content = box;

		चयन (event->तथ्य.caret.type) {
		हाल CONTENT_CARET_REMOVE:
		हाल CONTENT_CARET_HIDE:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, शून्य);
			अवरोध;
		हाल CONTENT_CARET_SET_POS:
			html_set_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->तथ्य.caret.pos.x,
					event->तथ्य.caret.pos.y,
					event->तथ्य.caret.pos.height,
					event->तथ्य.caret.pos.clip);
			अवरोध;
		}
	}
		अवरोध;

	हाल CONTENT_संदेश_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		जोड़ html_drag_owner drag_owner;
		drag_owner.content = box;

		चयन (event->तथ्य.drag.type) {
		हाल CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			अवरोध;
		हाल CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			अवरोध;
		हाल CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			अवरोध;
		}
		html_set_drag_type(c, drag_type, drag_owner,
				event->तथ्य.drag.rect);
	}
		अवरोध;

	हाल CONTENT_संदेश_SELECTION:
	{
		html_selection_type sel_type;
		जोड़ html_selection_owner sel_owner;

		अगर (event->तथ्य.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} अन्यथा {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_set_selection(c, sel_type, sel_owner,
				event->तथ्य.selection.read_only);
	}
		अवरोध;

	शेष:
		अवरोध;
	}

	अगर (c->base.status == CONTENT_STATUS_READY &&
	    c->base.active == 0 &&
	    (event->type == CONTENT_संदेश_LOADING ||
	     event->type == CONTENT_संदेश_DONE ||
	     event->type == CONTENT_संदेश_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.available_height);
		content_set_done(&c->base);
	} अन्यथा अगर (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_संदेश_DONE &&
		   box != शून्य &&
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
		अगर (ms_now > c->base.reformat_time) {
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

	वापस NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
अटल bool html_replace_object(काष्ठा content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	निश्चित(object != शून्य);
	निश्चित(object->box != शून्य);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	अगर (object->content != शून्य) {
		/* remove existing object */
		अगर (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->base.active);
		}

		hlcache_handle_release(object->content);
		object->content = शून्य;

		object->box->object = शून्य;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), शून्य,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	अगर (error != NSERROR_OK)
		वापस false;

	क्रम (page = c; page != शून्य; page = page->page) {
		page->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		page->base.status = CONTENT_STATUS_READY;
	}

	वापस true;
}

/**
 * schedule callback for object refresh
 */
अटल व्योम html_object_refresh(व्योम *p)
{
	काष्ठा content_html_object *object = p;
	nsurl *refresh_url;

	निश्चित(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	अगर (refresh_url == शून्य)
		वापस;

	content_invalidate_reuse_तथ्य(object->content);

	अगर (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, काष्ठा browser_window *bw)
{
	काष्ठा content_html_object *object, *अग्र;

	क्रम (object = html->object_list; object != शून्य; object = अग्र) {
		अग्र = object->अग्र;

		अगर (object->content == शून्य || object->box == शून्य)
			जारी;

		अगर (content_get_type(object->content) == CONTENT_NONE)
			जारी;

		content_open(object->content,
			     bw,
			     &html->base,
			     object->box->object_params);
	}
	वापस NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	काष्ठा content_html_object *object;

	क्रम (object = htmlc->object_list;
	     object != शून्य;
	     object = object->अग्र) {
		अगर (object->content == शून्य)
			जारी;

		चयन (content_get_status(object->content)) {
		हाल CONTENT_STATUS_DONE:
			/* already loaded: do nothing */
			अवरोध;

		हाल CONTENT_STATUS_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			अवरोध;

		शेष:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = शून्य;
			अगर (object->box != शून्य) {
				htmlc->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->base.active);
			}
			अवरोध;

		}
	}

	वापस NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	काष्ठा content_html_object *object, *अग्र;

	क्रम (object = html->object_list; object != शून्य; object = अग्र) {
		अग्र = object->अग्र;

		अगर (object->content == शून्य || object->box == शून्य)
			जारी;

		अगर (content_get_type(object->content) == CONTENT_NONE)
			जारी;

		अगर (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->schedule(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	वापस NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	जबतक (html->object_list != शून्य) {
		काष्ठा content_html_object *victim = html->object_list;

		अगर (victim->content != शून्य) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			अगर (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->schedule(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->अग्र;
		मुक्त(victim);
	}
	वापस NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  काष्ठा box *box,
		  content_type permitted_types,
		  bool background)
{
	काष्ठा content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	अगर (c->aborted)
		वापस true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = सुस्मृति(1, माप(काष्ठा content_html_object));
	अगर (object == शून्य) {
		वापस false;
	}

	अगर (box == शून्य) {
		object_callback = html_object_nobox_callback;
	} अन्यथा {
		object_callback = html_object_callback;
	}

	object->parent = (काष्ठा content *) c;
	object->अग्र = शून्य;
	object->content = शून्य;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->base),
					शून्य,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	अगर (error != NSERROR_OK) {
		मुक्त(object);
		वापस error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->अग्र = c->object_list;
	c->object_list = object;

	c->अंक_objects++;
	अगर (box != शून्य) {
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	वापस true;
}
