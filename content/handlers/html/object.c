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
بنية content_html_object *html_get_objects(hlcache_handle *h, بنانشانی صحيح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->عدد_objects;

	أرجع c->object_list;
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
	أرجع;
}

/**
 * Update a box whose content has completed rendering.
 */

ساكن فراغ
html_object_پورا(بنية box *box,
		 hlcache_handle *object,
		 bool background)
{
	بنية box *b;

	إذا (background) {
		box->background = object;
		أرجع;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	بدل (box->type) {
	حالة BOX_جدول:
		box->type = BOX_بلاک;
		اكسر;
	افتراضي:
		/* TODO: Any other box types need mapping? */
		اكسر;
	}

	إذا (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		لكل (b = box; b; b = b->ماپا)
			b->max_چوڑائی = UNKNOWN_MAX_چوڑائی;

		/* delete any clones of this box */
		طالما (box->اگلا && (box->اگلا->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->اگلا = box->اگلا->اگلا;
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
	حالة CONTENT_پیغام_ERROR:
		hlcache_handle_release(object);

		chobject->content = عدم;
		اكسر;

	افتراضي:
		اكسر;
	}

	أرجع NSERROR_OK;
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
	html_content *c = (html_content *) o->ماپا;
	صحيح x, y;
	بنية box *box;

	box = o->box;

	بدل (event->type) {
	حالة CONTENT_پیغام_LOADING:
		إذا (c->ادھار.حالت != CONTENT_حالت_LOADING && c->bw != عدم)
			content_open(object,
					c->bw, &c->ادھار,
					box->object_params);
		اكسر;

	حالة CONTENT_پیغام_READY:
		إذا (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_چوڑائی != UNKNOWN_MAX_چوڑائی ?
							box->چوڑائی : 0,
					box->max_چوڑائی != UNKNOWN_MAX_چوڑائی ?
							box->اچائی : 0);

			/* Adjust parent content for new object size */
			html_object_پورا(box, object, o->background);
			إذا (c->ادھار.حالت == CONTENT_حالت_READY ||
					c->ادھار.حالت == CONTENT_حالت_پورا)
				content__reformat(&c->ادھار, false,
						c->ادھار.دستیاب_چوڑائی,
						c->ادھار.دستیاب_اچائی);
		}
		اكسر;

	حالة CONTENT_پیغام_پورا:
		c->ادھار.سرگرم--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ادھار.سرگرم);

		html_object_پورا(box, object, o->background);

		إذا (c->ادھار.حالت != CONTENT_حالت_LOADING &&
				box->flags & REPLACE_DIM) {
			اتحاد content_پیغام_انکڑے انکڑے;

			إذا (c->had_initial_layout == false) {
				اكسر;
			}

			إذا (!box_visible(box))
				اكسر;

			box_coords(box, &x, &y);

			انکڑے.redraw.x = x + box->padding[بایاں_2];
			انکڑے.redraw.y = y + box->padding[چوٹی_2];
			انکڑے.redraw.چوڑائی = box->چوڑائی;
			انکڑے.redraw.اچائی = box->اچائی;

			content_نشر(&c->ادھار, CONTENT_پیغام_REDRAW, &انکڑے);
		}
		اكسر;

	حالة CONTENT_پیغام_ERROR:
		hlcache_handle_release(object);

		o->content = عدم;

		c->ادھار.سرگرم--;
		NSLOG(netsurf, INFO, "%d fetches active", c->ادھار.سرگرم);

		html_object_failed(box, c, o->background);

		اكسر;

	حالة CONTENT_پیغام_REDRAW:
		إذا (c->ادھار.حالت != CONTENT_حالت_LOADING) {
			اتحاد content_پیغام_انکڑے انکڑے = event->انکڑے;

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
				صحيح چوڑائی = box->padding[بایاں_2] + box->چوڑائی +
						box->padding[دایاں_2];
				صحيح اچائی = box->padding[چوٹی_2] + box->اچائی +
						box->padding[BOTTOM];
				صحيح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_چوڑائی(box->background);
				إذا (hunit == CSS_UNIT_PCT) {
					l = (چوڑائی - w) * hpos / INTTOFIX(100);
				} وإلا {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_اچائی(box->background);
				إذا (vunit == CSS_UNIT_PCT) {
					t = (اچائی - h) * vpos / INTTOFIX(100);
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
					انکڑے.redraw.x = 0;
					انکڑے.redraw.y = 0;
					انکڑے.redraw.چوڑائی = box->چوڑائی;
					انکڑے.redraw.اچائی = box->اچائی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
					انکڑے.redraw.x = 0;
					انکڑے.redraw.y += t;
					انکڑے.redraw.چوڑائی = box->چوڑائی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
					انکڑے.redraw.x += l;
					انکڑے.redraw.y = 0;
					انکڑے.redraw.اچائی = box->اچائی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
					انکڑے.redraw.x += l;
					انکڑے.redraw.y += t;
					اكسر;

				افتراضي:
					اكسر;
				}

				/* Add offset to box */
				انکڑے.redraw.x += x;
				انکڑے.redraw.y += y;

			} وإلا {
				/* Non-background case */
				صحيح w = content_get_چوڑائی(object);
				صحيح h = content_get_اچائی(object);

				إذا (w != 0 && box->چوڑائی != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					انکڑے.redraw.x = انکڑے.redraw.x *
							box->چوڑائی / w;
					انکڑے.redraw.چوڑائی =
							انکڑے.redraw.چوڑائی *
							box->چوڑائی / w;
				}

				إذا (h != 0 && box->اچائی != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					انکڑے.redraw.y = انکڑے.redraw.y *
							box->اچائی / h;
					انکڑے.redraw.اچائی =
							انکڑے.redraw.اچائی *
							box->اچائی / h;
				}

				انکڑے.redraw.x += x + box->padding[بایاں_2];
				انکڑے.redraw.y += y + box->padding[چوٹی_2];
			}

			content_نشر(&c->ادھار, CONTENT_پیغام_REDRAW, &انکڑے);
		}
		اكسر;

	حالة CONTENT_پیغام_REFRESH:
		إذا (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->ویلا_سارنی(event->انکڑے.delay * 1000,
					html_object_refresh, o);
		}

		اكسر;

	حالة CONTENT_پیغام_LINK:
		/* Don't care about favicons that aren't on top level content */
		اكسر;

	حالة CONTENT_پیغام_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->انکڑے.jsthread) = عدم;
		اكسر;

	حالة CONTENT_پیغام_GETDIMS:
		*(event->انکڑے.getdims.viewport_چوڑائی) =
				content_get_چوڑائی_2(&c->ادھار);
		*(event->انکڑے.getdims.viewport_اچائی) =
				content_get_اچائی_2(&c->ادھار);
		اكسر;

	حالة CONTENT_پیغام_SCROLL:
		إذا (box->scroll_x != عدم)
			scrollbar_سیٹ(box->scroll_x, event->انکڑے.scroll.x0,
					false);
		إذا (box->scroll_y != عدم)
			scrollbar_سیٹ(box->scroll_y, event->انکڑے.scroll.y0,
					false);
		اكسر;

	حالة CONTENT_پیغام_DRAGSAVE:
	{
		اتحاد content_پیغام_انکڑے پیغام_انکڑے;
		إذا (event->انکڑے.dragsave.content == عدم)
			پیغام_انکڑے.dragsave.content = object;
		وإلا
			پیغام_انکڑے.dragsave.content =
					event->انکڑے.dragsave.content;

		content_نشر(&c->ادھار, CONTENT_پیغام_DRAGSAVE, &پیغام_انکڑے);
	}
		اكسر;

	حالة CONTENT_پیغام_SAVELINK:
	حالة CONTENT_پیغام_اشارہ_گر:
	حالة CONTENT_پیغام_SELECTMENU:
	حالة CONTENT_پیغام_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_نشر(&c->ادھار, event->type, &event->انکڑے);
		اكسر;

	حالة CONTENT_پیغام_CARET:
	{
		اتحاد html_focus_owner focus_owner;
		focus_owner.content = box;

		بدل (event->انکڑے.caret.type) {
		حالة CONTENT_CARET_REMOVE:
		حالة CONTENT_CARET_HIDE:
			html_سیٹ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, عدم);
			اكسر;
		حالة CONTENT_CARET_سیٹ_POS:
			html_سیٹ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->انکڑے.caret.pos.x,
					event->انکڑے.caret.pos.y,
					event->انکڑے.caret.pos.اچائی,
					event->انکڑے.caret.pos.clip);
			اكسر;
		}
	}
		اكسر;

	حالة CONTENT_پیغام_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اتحاد html_drag_owner drag_owner;
		drag_owner.content = box;

		بدل (event->انکڑے.drag.type) {
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
		html_سیٹ_drag_type(c, drag_type, drag_owner,
				event->انکڑے.drag.rect);
	}
		اكسر;

	حالة CONTENT_پیغام_SELECTION:
	{
		html_selection_type sel_type;
		اتحاد html_selection_owner sel_owner;

		إذا (event->انکڑے.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} وإلا {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_سیٹ_selection(c, sel_type, sel_owner,
				event->انکڑے.selection.read_only);
	}
		اكسر;

	افتراضي:
		اكسر;
	}

	إذا (c->ادھار.حالت == CONTENT_حالت_READY &&
	    c->ادھار.سرگرم == 0 &&
	    (event->type == CONTENT_پیغام_LOADING ||
	     event->type == CONTENT_پیغام_پورا ||
	     event->type == CONTENT_پیغام_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->ادھار, false, c->ادھار.دستیاب_چوڑائی,
				c->ادھار.دستیاب_اچائی);
		content_سیٹ_پورا(&c->ادھار);
	} وإلا إذا (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_پیغام_پورا &&
		   box != عدم &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->ادھار.حالت == CONTENT_حالت_READY ||
		    c->ادھار.حالت == CONTENT_حالت_پورا)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		إذا (ms_now > c->ادھار.reformat_ویلا) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->ادھار,
					  false,
					  c->ادھار.دستیاب_چوڑائی,
					  c->ادھار.دستیاب_اچائی);
		}
	}

	أرجع NSERROR_OK;
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
	hlcache_بچہ_context بچہ;
	html_content *ورقہ;
	nserror error;

	أكد(object != عدم);
	أكد(object->box != عدم);

	c = (html_content *) object->ماپا;

	بچہ.charset = c->encoding;
	بچہ.quirks = c->ادھار.quirks;

	إذا (object->content != عدم) {
		/* remove existing object */
		إذا (content_get_حالت(object->content) != CONTENT_حالت_پورا) {
			c->ادھار.سرگرم--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->ادھار.سرگرم);
		}

		hlcache_handle_release(object->content);
		object->content = عدم;

		object->box->object = عدم;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->ادھار), عدم,
			html_object_callback, object, &بچہ,
			object->permitted_types,
			&object->content);

	إذا (error != NSERROR_OK)
		أرجع false;

	لكل (ورقہ = c; ورقہ != عدم; ورقہ = ورقہ->ورقہ) {
		ورقہ->ادھار.سرگرم++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ادھار.سرگرم);

		ورقہ->ادھار.حالت = CONTENT_حالت_READY;
	}

	أرجع true;
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
		أرجع;

	content_invalidate_reuse_انکڑے(object->content);

	إذا (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, بنية browser_کھڑکی *bw)
{
	بنية content_html_object *object, *اگلا;

	لكل (object = html->object_list; object != عدم; object = اگلا) {
		اگلا = object->اگلا;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		content_open(object->content,
			     bw,
			     &html->ادھار,
			     object->box->object_params);
	}
	أرجع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	بنية content_html_object *object;

	لكل (object = htmlc->object_list;
	     object != عدم;
	     object = object->اگلا) {
		إذا (object->content == عدم)
			استمر;

		بدل (content_get_حالت(object->content)) {
		حالة CONTENT_حالت_پورا:
			/* already loaded: do nothing */
			اكسر;

		حالة CONTENT_حالت_READY:
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
				htmlc->ادھار.سرگرم--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->ادھار.سرگرم);
			}
			اكسر;

		}
	}

	أرجع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	بنية content_html_object *object, *اگلا;

	لكل (object = html->object_list; object != عدم; object = اگلا) {
		اگلا = object->اگلا;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->ویلا_سارنی(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	أرجع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	طالما (html->object_list != عدم) {
		بنية content_html_object *victim = html->object_list;

		إذا (victim->content != عدم) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			إذا (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->ویلا_سارنی(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->اگلا;
		حرر(victim);
	}
	أرجع NSERROR_OK;
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
	hlcache_بچہ_context بچہ;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	إذا (c->aborted)
		أرجع true;

	بچہ.charset = c->encoding;
	بچہ.quirks = c->ادھار.quirks;

	object = صفر_حافظہ_دیو(1, حجم(بنية content_html_object));
	إذا (object == عدم) {
		أرجع false;
	}

	إذا (box == عدم) {
		object_callback = html_object_nobox_callback;
	} وإلا {
		object_callback = html_object_callback;
	}

	object->ماپا = (بنية content *) c;
	object->اگلا = عدم;
	object->content = عدم;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->ادھار),
					عدم,
					object_callback,
					object,
					&بچہ,
					object->permitted_types,
					&object->content);
	إذا (error != NSERROR_OK) {
		حرر(object);
		أرجع error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->اگلا = c->object_list;
	c->object_list = object;

	c->عدد_objects++;
	إذا (box != عدم) {
		c->ادھار.سرگرم++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ادھار.سرگرم);
	}

	أرجع true;
}
