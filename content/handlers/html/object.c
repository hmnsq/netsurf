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
بنية content_html_object *html_get_objects(hlcache_handle *h, بیعلامت صحيح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->رقم_objects;

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
html_object_انجامشده(بنية box *box,
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
		box->type = BOX_بلوک;
		اكسر;
	افتراضي:
		/* TODO: Any other box types need mapping? */
		اكسر;
	}

	إذا (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		لكل (b = box; b; b = b->والد)
			b->max_عرض = UNKNOWN_MAX_عرض;

		/* delete any clones of this box */
		طالما (box->بعدی && (box->بعدی->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->بعدی = box->بعدی->بعدی;
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
	حالة CONTENT_پیام_ERROR:
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
	html_content *c = (html_content *) o->والد;
	صحيح x, y;
	بنية box *box;

	box = o->box;

	بدل (event->type) {
	حالة CONTENT_پیام_LOADING:
		إذا (c->پایه.وضعیت != CONTENT_وضعیت_LOADING && c->bw != عدم)
			content_open(object,
					c->bw, &c->پایه,
					box->object_params);
		اكسر;

	حالة CONTENT_پیام_READY:
		إذا (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_عرض != UNKNOWN_MAX_عرض ?
							box->عرض : 0,
					box->max_عرض != UNKNOWN_MAX_عرض ?
							box->ارتفاع : 0);

			/* Adjust parent content for new object size */
			html_object_انجامشده(box, object, o->background);
			إذا (c->پایه.وضعیت == CONTENT_وضعیت_READY ||
					c->پایه.وضعیت == CONTENT_وضعیت_انجامشده)
				content__reformat(&c->پایه, false,
						c->پایه.موجود_عرض,
						c->پایه.موجود_ارتفاع);
		}
		اكسر;

	حالة CONTENT_پیام_انجامشده:
		c->پایه.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->پایه.فعال);

		html_object_انجامشده(box, object, o->background);

		إذا (c->پایه.وضعیت != CONTENT_وضعیت_LOADING &&
				box->flags & REPLACE_DIM) {
			اتحاد content_پیام_داده داده;

			إذا (c->had_initial_layout == false) {
				اكسر;
			}

			إذا (!box_visible(box))
				اكسر;

			box_coords(box, &x, &y);

			داده.redraw.x = x + box->padding[چپ_2];
			داده.redraw.y = y + box->padding[بالاترین_2];
			داده.redraw.عرض = box->عرض;
			داده.redraw.ارتفاع = box->ارتفاع;

			content_پخش(&c->پایه, CONTENT_پیام_REDRAW, &داده);
		}
		اكسر;

	حالة CONTENT_پیام_ERROR:
		hlcache_handle_release(object);

		o->content = عدم;

		c->پایه.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->پایه.فعال);

		html_object_failed(box, c, o->background);

		اكسر;

	حالة CONTENT_پیام_REDRAW:
		إذا (c->پایه.وضعیت != CONTENT_وضعیت_LOADING) {
			اتحاد content_پیام_داده داده = event->داده;

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
				صحيح عرض = box->padding[چپ_2] + box->عرض +
						box->padding[راست_2];
				صحيح ارتفاع = box->padding[بالاترین_2] + box->ارتفاع +
						box->padding[BOTTOM];
				صحيح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_عرض(box->background);
				إذا (hunit == CSS_UNIT_PCT) {
					l = (عرض - w) * hpos / INTTOFIX(100);
				} وإلا {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_ارتفاع(box->background);
				إذا (vunit == CSS_UNIT_PCT) {
					t = (ارتفاع - h) * vpos / INTTOFIX(100);
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
					داده.redraw.x = 0;
					داده.redraw.y = 0;
					داده.redraw.عرض = box->عرض;
					داده.redraw.ارتفاع = box->ارتفاع;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
					داده.redraw.x = 0;
					داده.redraw.y += t;
					داده.redraw.عرض = box->عرض;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
					داده.redraw.x += l;
					داده.redraw.y = 0;
					داده.redraw.ارتفاع = box->ارتفاع;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
					داده.redraw.x += l;
					داده.redraw.y += t;
					اكسر;

				افتراضي:
					اكسر;
				}

				/* Add offset to box */
				داده.redraw.x += x;
				داده.redraw.y += y;

			} وإلا {
				/* Non-background case */
				صحيح w = content_get_عرض(object);
				صحيح h = content_get_ارتفاع(object);

				إذا (w != 0 && box->عرض != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					داده.redraw.x = داده.redraw.x *
							box->عرض / w;
					داده.redraw.عرض =
							داده.redraw.عرض *
							box->عرض / w;
				}

				إذا (h != 0 && box->ارتفاع != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					داده.redraw.y = داده.redraw.y *
							box->ارتفاع / h;
					داده.redraw.ارتفاع =
							داده.redraw.ارتفاع *
							box->ارتفاع / h;
				}

				داده.redraw.x += x + box->padding[چپ_2];
				داده.redraw.y += y + box->padding[بالاترین_2];
			}

			content_پخش(&c->پایه, CONTENT_پیام_REDRAW, &داده);
		}
		اكسر;

	حالة CONTENT_پیام_REFRESH:
		إذا (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->زمانبندی(event->داده.delay * 1000,
					html_object_refresh, o);
		}

		اكسر;

	حالة CONTENT_پیام_LINK:
		/* Don't care about favicons that aren't on top level content */
		اكسر;

	حالة CONTENT_پیام_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->داده.jsthread) = عدم;
		اكسر;

	حالة CONTENT_پیام_GETDIMS:
		*(event->داده.getdims.viewport_عرض) =
				content_get_عرض_2(&c->پایه);
		*(event->داده.getdims.viewport_ارتفاع) =
				content_get_ارتفاع_2(&c->پایه);
		اكسر;

	حالة CONTENT_پیام_SCROLL:
		إذا (box->scroll_x != عدم)
			scrollbar_مجموعه(box->scroll_x, event->داده.scroll.x0,
					false);
		إذا (box->scroll_y != عدم)
			scrollbar_مجموعه(box->scroll_y, event->داده.scroll.y0,
					false);
		اكسر;

	حالة CONTENT_پیام_DRAGSAVE:
	{
		اتحاد content_پیام_داده پیام_داده;
		إذا (event->داده.dragsave.content == عدم)
			پیام_داده.dragsave.content = object;
		وإلا
			پیام_داده.dragsave.content =
					event->داده.dragsave.content;

		content_پخش(&c->پایه, CONTENT_پیام_DRAGSAVE, &پیام_داده);
	}
		اكسر;

	حالة CONTENT_پیام_SAVELINK:
	حالة CONTENT_پیام_اشاره_گر:
	حالة CONTENT_پیام_SELECTMENU:
	حالة CONTENT_پیام_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_پخش(&c->پایه, event->type, &event->داده);
		اكسر;

	حالة CONTENT_پیام_CARET:
	{
		اتحاد html_focus_owner focus_owner;
		focus_owner.content = box;

		بدل (event->داده.caret.type) {
		حالة CONTENT_CARET_REMOVE:
		حالة CONTENT_CARET_HIDE:
			html_مجموعه_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, عدم);
			اكسر;
		حالة CONTENT_CARET_مجموعه_POS:
			html_مجموعه_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->داده.caret.pos.x,
					event->داده.caret.pos.y,
					event->داده.caret.pos.ارتفاع,
					event->داده.caret.pos.clip);
			اكسر;
		}
	}
		اكسر;

	حالة CONTENT_پیام_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اتحاد html_drag_owner drag_owner;
		drag_owner.content = box;

		بدل (event->داده.drag.type) {
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
		html_مجموعه_drag_type(c, drag_type, drag_owner,
				event->داده.drag.rect);
	}
		اكسر;

	حالة CONTENT_پیام_SELECTION:
	{
		html_selection_type sel_type;
		اتحاد html_selection_owner sel_owner;

		إذا (event->داده.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} وإلا {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_مجموعه_selection(c, sel_type, sel_owner,
				event->داده.selection.read_only);
	}
		اكسر;

	افتراضي:
		اكسر;
	}

	إذا (c->پایه.وضعیت == CONTENT_وضعیت_READY &&
	    c->پایه.فعال == 0 &&
	    (event->type == CONTENT_پیام_LOADING ||
	     event->type == CONTENT_پیام_انجامشده ||
	     event->type == CONTENT_پیام_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->پایه, false, c->پایه.موجود_عرض,
				c->پایه.موجود_ارتفاع);
		content_مجموعه_انجامشده(&c->پایه);
	} وإلا إذا (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_پیام_انجامشده &&
		   box != عدم &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->پایه.وضعیت == CONTENT_وضعیت_READY ||
		    c->پایه.وضعیت == CONTENT_وضعیت_انجامشده)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		إذا (ms_now > c->پایه.reformat_زمان) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->پایه,
					  false,
					  c->پایه.موجود_عرض,
					  c->پایه.موجود_ارتفاع);
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
	hlcache_فرزند_context فرزند;
	html_content *صفحه;
	nserror error;

	أكد(object != عدم);
	أكد(object->box != عدم);

	c = (html_content *) object->والد;

	فرزند.charset = c->encoding;
	فرزند.quirks = c->پایه.quirks;

	إذا (object->content != عدم) {
		/* remove existing object */
		إذا (content_get_وضعیت(object->content) != CONTENT_وضعیت_انجامشده) {
			c->پایه.فعال--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->پایه.فعال);
		}

		hlcache_handle_release(object->content);
		object->content = عدم;

		object->box->object = عدم;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->پایه), عدم,
			html_object_callback, object, &فرزند,
			object->permitted_types,
			&object->content);

	إذا (error != NSERROR_OK)
		أرجع false;

	لكل (صفحه = c; صفحه != عدم; صفحه = صفحه->صفحه) {
		صفحه->پایه.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->پایه.فعال);

		صفحه->پایه.وضعیت = CONTENT_وضعیت_READY;
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

	content_invalidate_reuse_داده(object->content);

	إذا (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, بنية browser_پنجره *bw)
{
	بنية content_html_object *object, *بعدی;

	لكل (object = html->object_list; object != عدم; object = بعدی) {
		بعدی = object->بعدی;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		content_open(object->content,
			     bw,
			     &html->پایه,
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
	     object = object->بعدی) {
		إذا (object->content == عدم)
			استمر;

		بدل (content_get_وضعیت(object->content)) {
		حالة CONTENT_وضعیت_انجامشده:
			/* already loaded: do nothing */
			اكسر;

		حالة CONTENT_وضعیت_READY:
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
				htmlc->پایه.فعال--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->پایه.فعال);
			}
			اكسر;

		}
	}

	أرجع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	بنية content_html_object *object, *بعدی;

	لكل (object = html->object_list; object != عدم; object = بعدی) {
		بعدی = object->بعدی;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->زمانبندی(-1, html_object_refresh, object);
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
				guit->misc->زمانبندی(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->بعدی;
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
	hlcache_فرزند_context فرزند;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	إذا (c->aborted)
		أرجع true;

	فرزند.charset = c->encoding;
	فرزند.quirks = c->پایه.quirks;

	object = حافظه_صفر_بده(1, حجم(بنية content_html_object));
	إذا (object == عدم) {
		أرجع false;
	}

	إذا (box == عدم) {
		object_callback = html_object_nobox_callback;
	} وإلا {
		object_callback = html_object_callback;
	}

	object->والد = (بنية content *) c;
	object->بعدی = عدم;
	object->content = عدم;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->پایه),
					عدم,
					object_callback,
					object,
					&فرزند,
					object->permitted_types,
					&object->content);
	إذا (error != NSERROR_OK) {
		حرر(object);
		أرجع error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->بعدی = c->object_list;
	c->object_list = object;

	c->رقم_objects++;
	إذا (box != عدم) {
		c->پایه.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->پایه.فعال);
	}

	أرجع true;
}
