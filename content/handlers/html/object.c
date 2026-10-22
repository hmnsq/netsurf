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
संरचना content_html_object *html_get_objects(hlcache_handle *h, अचिह्नित पूर्णाङ्क *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->अङ्कः_objects;

	प्रत्यागम c->object_list;
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
	प्रत्यागम;
}

/**
 * Update a box whose content has completed rendering.
 */

स्थिर शून्य
html_object_समाप्तम्(संरचना box *box,
		 hlcache_handle *object,
		 bool background)
{
	संरचना box *b;

	यदि (background) {
		box->background = object;
		प्रत्यागम;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	विकल्प (box->type) {
	पक्ष BOX_सारणी:
		box->type = BOX_खण्डम्;
		भङ्ग;
	स्वभाव:
		/* TODO: Any other box types need mapping? */
		भङ्ग;
	}

	यदि (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		प्रति (b = box; b; b = b->parent)
			b->max_विस्तारः = UNKNOWN_MAX_विस्तारः;

		/* delete any clones of this box */
		यावत् (box->अग्र && (box->अग्र->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->अग्र = box->अग्र->अग्र;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
स्थिर nserror
html_object_nobox_callback(hlcache_handle *object,
			   ध्रुव hlcache_event *event,
			   शून्य *pw)
{
	संरचना content_html_object *chobject = pw;

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देशः_ERROR:
		hlcache_handle_release(object);

		chobject->content = रिक्त;
		भङ्ग;

	स्वभाव:
		भङ्ग;
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
स्थिर nserror
html_object_callback(hlcache_handle *object,
		     ध्रुव hlcache_event *event,
		     शून्य *pw)
{
	संरचना content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	पूर्णाङ्क x, y;
	संरचना box *box;

	box = o->box;

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देशः_LOADING:
		यदि (c->आधारः.स्थितिः != CONTENT_स्थितिः_LOADING && c->bw != रिक्त)
			content_open(object,
					c->bw, &c->आधारः,
					box->object_params);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_READY:
		यदि (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_विस्तारः != UNKNOWN_MAX_विस्तारः ?
							box->विस्तारः : 0,
					box->max_विस्तारः != UNKNOWN_MAX_विस्तारः ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_समाप्तम्(box, object, o->background);
			यदि (c->आधारः.स्थितिः == CONTENT_स्थितिः_READY ||
					c->आधारः.स्थितिः == CONTENT_स्थितिः_समाप्तम्)
				content__reformat(&c->आधारः, false,
						c->आधारः.उपलब्धम्_विस्तारः,
						c->आधारः.उपलब्धम्_height);
		}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_समाप्तम्:
		c->आधारः.सक्रिय--;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

		html_object_समाप्तम्(box, object, o->background);

		यदि (c->आधारः.स्थितिः != CONTENT_स्थितिः_LOADING &&
				box->flags & REPLACE_DIM) {
			सङ्घ content_सन्देशः_दत्त दत्त;

			यदि (c->had_initial_layout == false) {
				भङ्ग;
			}

			यदि (!box_visible(box))
				भङ्ग;

			box_coords(box, &x, &y);

			दत्त.redraw.x = x + box->padding[वामः_2];
			दत्त.redraw.y = y + box->padding[शिखरम्_2];
			दत्त.redraw.विस्तारः = box->विस्तारः;
			दत्त.redraw.height = box->height;

			content_प्रसारणम्(&c->आधारः, CONTENT_सन्देशः_REDRAW, &दत्त);
		}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		hlcache_handle_release(object);

		o->content = रिक्त;

		c->आधारः.सक्रिय--;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

		html_object_failed(box, c, o->background);

		भङ्ग;

	पक्ष CONTENT_सन्देशः_REDRAW:
		यदि (c->आधारः.स्थितिः != CONTENT_स्थितिः_LOADING) {
			सङ्घ content_सन्देशः_दत्त दत्त = event->दत्त;

			यदि (c->had_initial_layout == false) {
				भङ्ग;
			}

			यदि (!box_visible(box))
				भङ्ग;

			box_coords(box, &x, &y);

			यदि (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				पूर्णाङ्क विस्तारः = box->padding[वामः_2] + box->विस्तारः +
						box->padding[दक्षिणः_2];
				पूर्णाङ्क height = box->padding[शिखरम्_2] + box->height +
						box->padding[BOTTOM];
				पूर्णाङ्क t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_विस्तारः(box->background);
				यदि (hunit == CSS_UNIT_PCT) {
					l = (विस्तारः - w) * hpos / INTTOFIX(100);
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
					दत्त.redraw.x = 0;
					दत्त.redraw.y = 0;
					दत्त.redraw.विस्तारः = box->विस्तारः;
					दत्त.redraw.height = box->height;
					भङ्ग;

				पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
					दत्त.redraw.x = 0;
					दत्त.redraw.y += t;
					दत्त.redraw.विस्तारः = box->विस्तारः;
					भङ्ग;

				पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
					दत्त.redraw.x += l;
					दत्त.redraw.y = 0;
					दत्त.redraw.height = box->height;
					भङ्ग;

				पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
					दत्त.redraw.x += l;
					दत्त.redraw.y += t;
					भङ्ग;

				स्वभाव:
					भङ्ग;
				}

				/* Add offset to box */
				दत्त.redraw.x += x;
				दत्त.redraw.y += y;

			} अन्यथा {
				/* Non-background case */
				पूर्णाङ्क w = content_get_विस्तारः(object);
				पूर्णाङ्क h = content_get_height(object);

				यदि (w != 0 && box->विस्तारः != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					दत्त.redraw.x = दत्त.redraw.x *
							box->विस्तारः / w;
					दत्त.redraw.विस्तारः =
							दत्त.redraw.विस्तारः *
							box->विस्तारः / w;
				}

				यदि (h != 0 && box->height != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					दत्त.redraw.y = दत्त.redraw.y *
							box->height / h;
					दत्त.redraw.height =
							दत्त.redraw.height *
							box->height / h;
				}

				दत्त.redraw.x += x + box->padding[वामः_2];
				दत्त.redraw.y += y + box->padding[शिखरम्_2];
			}

			content_प्रसारणम्(&c->आधारः, CONTENT_सन्देशः_REDRAW, &दत्त);
		}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_REFRESH:
		यदि (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->कालसारणी(event->दत्त.delay * 1000,
					html_object_refresh, o);
		}

		भङ्ग;

	पक्ष CONTENT_सन्देशः_LINK:
		/* Don't care about favicons that aren't on top level content */
		भङ्ग;

	पक्ष CONTENT_सन्देशः_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->दत्त.jsthread) = रिक्त;
		भङ्ग;

	पक्ष CONTENT_सन्देशः_GETDIMS:
		*(event->दत्त.getdims.viewport_विस्तारः) =
				content_get_विस्तारः_2(&c->आधारः);
		*(event->दत्त.getdims.viewport_height) =
				content__get_height(&c->आधारः);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_SCROLL:
		यदि (box->scroll_x != रिक्त)
			scrollbar_समुच्चयः(box->scroll_x, event->दत्त.scroll.x0,
					false);
		यदि (box->scroll_y != रिक्त)
			scrollbar_समुच्चयः(box->scroll_y, event->दत्त.scroll.y0,
					false);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_DRAGSAVE:
	{
		सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
		यदि (event->दत्त.dragsave.content == रिक्त)
			सन्देशः_दत्त.dragsave.content = object;
		अन्यथा
			सन्देशः_दत्त.dragsave.content =
					event->दत्त.dragsave.content;

		content_प्रसारणम्(&c->आधारः, CONTENT_सन्देशः_DRAGSAVE, &सन्देशः_दत्त);
	}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_SAVELINK:
	पक्ष CONTENT_सन्देशः_POINTER:
	पक्ष CONTENT_सन्देशः_SELECTMENU:
	पक्ष CONTENT_सन्देशः_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_प्रसारणम्(&c->आधारः, event->type, &event->दत्त);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_CARET:
	{
		सङ्घ html_focus_owner focus_owner;
		focus_owner.content = box;

		विकल्प (event->दत्त.caret.type) {
		पक्ष CONTENT_CARET_REMOVE:
		पक्ष CONTENT_CARET_HIDE:
			html_समुच्चयः_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, रिक्त);
			भङ्ग;
		पक्ष CONTENT_CARET_समुच्चयः_POS:
			html_समुच्चयः_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->दत्त.caret.pos.x,
					event->दत्त.caret.pos.y,
					event->दत्त.caret.pos.height,
					event->दत्त.caret.pos.clip);
			भङ्ग;
		}
	}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		सङ्घ html_drag_owner drag_owner;
		drag_owner.content = box;

		विकल्प (event->दत्त.drag.type) {
		पक्ष CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			भङ्ग;
		पक्ष CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			भङ्ग;
		पक्ष CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			भङ्ग;
		}
		html_समुच्चयः_drag_type(c, drag_type, drag_owner,
				event->दत्त.drag.rect);
	}
		भङ्ग;

	पक्ष CONTENT_सन्देशः_SELECTION:
	{
		html_selection_type sel_type;
		सङ्घ html_selection_owner sel_owner;

		यदि (event->दत्त.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} अन्यथा {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_समुच्चयः_selection(c, sel_type, sel_owner,
				event->दत्त.selection.read_only);
	}
		भङ्ग;

	स्वभाव:
		भङ्ग;
	}

	यदि (c->आधारः.स्थितिः == CONTENT_स्थितिः_READY &&
	    c->आधारः.सक्रिय == 0 &&
	    (event->type == CONTENT_सन्देशः_LOADING ||
	     event->type == CONTENT_सन्देशः_समाप्तम् ||
	     event->type == CONTENT_सन्देशः_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->आधारः, false, c->आधारः.उपलब्धम्_विस्तारः,
				c->आधारः.उपलब्धम्_height);
		content_समुच्चयः_समाप्तम्(&c->आधारः);
	} अन्यथा यदि (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_सन्देशः_समाप्तम् &&
		   box != रिक्त &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->आधारः.स्थितिः == CONTENT_स्थितिः_READY ||
		    c->आधारः.स्थितिः == CONTENT_स्थितिः_समाप्तम्)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		यदि (ms_now > c->आधारः.reformat_time) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->आधारः,
					  false,
					  c->आधारः.उपलब्धम्_विस्तारः,
					  c->आधारः.उपलब्धम्_height);
		}
	}

	प्रत्यागम NSERROR_OK;
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
	child.quirks = c->आधारः.quirks;

	यदि (object->content != रिक्त) {
		/* remove existing object */
		यदि (content_get_स्थितिः(object->content) != CONTENT_स्थितिः_समाप्तम्) {
			c->आधारः.सक्रिय--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->आधारः.सक्रिय);
		}

		hlcache_handle_release(object->content);
		object->content = रिक्त;

		object->box->object = रिक्त;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->आधारः), रिक्त,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	यदि (error != NSERROR_OK)
		प्रत्यागम false;

	प्रति (page = c; page != रिक्त; page = page->page) {
		page->आधारः.सक्रिय++;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

		page->आधारः.स्थितिः = CONTENT_स्थितिः_READY;
	}

	प्रत्यागम true;
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
		प्रत्यागम;

	content_invalidate_reuse_दत्त(object->content);

	यदि (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, संरचना browser_गवाक्षः *bw)
{
	संरचना content_html_object *object, *अग्र;

	प्रति (object = html->object_list; object != रिक्त; object = अग्र) {
		अग्र = object->अग्र;

		यदि (object->content == रिक्त || object->box == रिक्त)
			अनुवर्त;

		यदि (content_get_type(object->content) == CONTENT_NONE)
			अनुवर्त;

		content_open(object->content,
			     bw,
			     &html->आधारः,
			     object->box->object_params);
	}
	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	संरचना content_html_object *object;

	प्रति (object = htmlc->object_list;
	     object != रिक्त;
	     object = object->अग्र) {
		यदि (object->content == रिक्त)
			अनुवर्त;

		विकल्प (content_get_स्थितिः(object->content)) {
		पक्ष CONTENT_स्थितिः_समाप्तम्:
			/* already loaded: do nothing */
			भङ्ग;

		पक्ष CONTENT_स्थितिः_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			भङ्ग;

		स्वभाव:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = रिक्त;
			यदि (object->box != रिक्त) {
				htmlc->आधारः.सक्रिय--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->आधारः.सक्रिय);
			}
			भङ्ग;

		}
	}

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	संरचना content_html_object *object, *अग्र;

	प्रति (object = html->object_list; object != रिक्त; object = अग्र) {
		अग्र = object->अग्र;

		यदि (object->content == रिक्त || object->box == रिक्त)
			अनुवर्त;

		यदि (content_get_type(object->content) == CONTENT_NONE)
			अनुवर्त;

		यदि (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->कालसारणी(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	यावत् (html->object_list != रिक्त) {
		संरचना content_html_object *victim = html->object_list;

		यदि (victim->content != रिक्त) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			यदि (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->कालसारणी(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->अग्र;
		मुक्त(victim);
	}
	प्रत्यागम NSERROR_OK;
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
		प्रत्यागम true;

	child.charset = c->encoding;
	child.quirks = c->आधारः.quirks;

	object = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content_html_object));
	यदि (object == रिक्त) {
		प्रत्यागम false;
	}

	यदि (box == रिक्त) {
		object_callback = html_object_nobox_callback;
	} अन्यथा {
		object_callback = html_object_callback;
	}

	object->parent = (संरचना content *) c;
	object->अग्र = रिक्त;
	object->content = रिक्त;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->आधारः),
					रिक्त,
					object_callback,
					object,
					&child,
					object->permitted_types,
					&object->content);
	यदि (error != NSERROR_OK) {
		मुक्त(object);
		प्रत्यागम error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->अग्र = c->object_list;
	c->object_list = object;

	c->अङ्कः_objects++;
	यदि (box != रिक्त) {
		c->आधारः.सक्रिय++;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);
	}

	प्रत्यागम true;
}
