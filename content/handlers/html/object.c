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
بنية content_html_object *html_get_objects(hlcache_handle *h, بینښه صحيح *n)
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
html_object_بشپړ(بنية box *box,
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
		لكل (b = box; b; b = b->پلار)
			b->max_پلنوالی = UNKNOWN_MAX_پلنوالی;

		/* delete any clones of this box */
		طالما (box->بل && (box->بل->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->بل = box->بل->بل;
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
	html_content *c = (html_content *) o->پلار;
	صحيح x, y;
	بنية box *box;

	box = o->box;

	بدل (event->type) {
	حالة CONTENT_پیغام_LOADING:
		إذا (c->بنسټ.حالت != CONTENT_حالت_LOADING && c->bw != عدم)
			content_open(object,
					c->bw, &c->بنسټ,
					box->object_params);
		اكسر;

	حالة CONTENT_پیغام_READY:
		إذا (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_پلنوالی != UNKNOWN_MAX_پلنوالی ?
							box->پلنوالی : 0,
					box->max_پلنوالی != UNKNOWN_MAX_پلنوالی ?
							box->لوړوالی : 0);

			/* Adjust parent content for new object size */
			html_object_بشپړ(box, object, o->background);
			إذا (c->بنسټ.حالت == CONTENT_حالت_READY ||
					c->بنسټ.حالت == CONTENT_حالت_بشپړ)
				content__reformat(&c->بنسټ, false,
						c->بنسټ.شته_پلنوالی,
						c->بنسټ.شته_لوړوالی);
		}
		اكسر;

	حالة CONTENT_پیغام_بشپړ:
		c->بنسټ.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

		html_object_بشپړ(box, object, o->background);

		إذا (c->بنسټ.حالت != CONTENT_حالت_LOADING &&
				box->flags & REPLACE_DIM) {
			اتحاد content_پیغام_معلومات معلومات;

			إذا (c->had_initial_layout == false) {
				اكسر;
			}

			إذا (!box_visible(box))
				اكسر;

			box_coords(box, &x, &y);

			معلومات.redraw.x = x + box->padding[کیڼ_2];
			معلومات.redraw.y = y + box->padding[سر_2];
			معلومات.redraw.پلنوالی = box->پلنوالی;
			معلومات.redraw.لوړوالی = box->لوړوالی;

			content_خپرونه(&c->بنسټ, CONTENT_پیغام_REDRAW, &معلومات);
		}
		اكسر;

	حالة CONTENT_پیغام_ERROR:
		hlcache_handle_release(object);

		o->content = عدم;

		c->بنسټ.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

		html_object_failed(box, c, o->background);

		اكسر;

	حالة CONTENT_پیغام_REDRAW:
		إذا (c->بنسټ.حالت != CONTENT_حالت_LOADING) {
			اتحاد content_پیغام_معلومات معلومات = event->معلومات;

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
				صحيح پلنوالی = box->padding[کیڼ_2] + box->پلنوالی +
						box->padding[ښی_2];
				صحيح لوړوالی = box->padding[سر_2] + box->لوړوالی +
						box->padding[BOTTOM];
				صحيح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_پلنوالی(box->background);
				إذا (hunit == CSS_UNIT_PCT) {
					l = (پلنوالی - w) * hpos / INTTOFIX(100);
				} وإلا {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_لوړوالی(box->background);
				إذا (vunit == CSS_UNIT_PCT) {
					t = (لوړوالی - h) * vpos / INTTOFIX(100);
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
					معلومات.redraw.x = 0;
					معلومات.redraw.y = 0;
					معلومات.redraw.پلنوالی = box->پلنوالی;
					معلومات.redraw.لوړوالی = box->لوړوالی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
					معلومات.redraw.x = 0;
					معلومات.redraw.y += t;
					معلومات.redraw.پلنوالی = box->پلنوالی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
					معلومات.redraw.x += l;
					معلومات.redraw.y = 0;
					معلومات.redraw.لوړوالی = box->لوړوالی;
					اكسر;

				حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
					معلومات.redraw.x += l;
					معلومات.redraw.y += t;
					اكسر;

				افتراضي:
					اكسر;
				}

				/* Add offset to box */
				معلومات.redraw.x += x;
				معلومات.redraw.y += y;

			} وإلا {
				/* Non-background case */
				صحيح w = content_get_پلنوالی(object);
				صحيح h = content_get_لوړوالی(object);

				إذا (w != 0 && box->پلنوالی != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					معلومات.redraw.x = معلومات.redraw.x *
							box->پلنوالی / w;
					معلومات.redraw.پلنوالی =
							معلومات.redraw.پلنوالی *
							box->پلنوالی / w;
				}

				إذا (h != 0 && box->لوړوالی != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					معلومات.redraw.y = معلومات.redraw.y *
							box->لوړوالی / h;
					معلومات.redraw.لوړوالی =
							معلومات.redraw.لوړوالی *
							box->لوړوالی / h;
				}

				معلومات.redraw.x += x + box->padding[کیڼ_2];
				معلومات.redraw.y += y + box->padding[سر_2];
			}

			content_خپرونه(&c->بنسټ, CONTENT_پیغام_REDRAW, &معلومات);
		}
		اكسر;

	حالة CONTENT_پیغام_REFRESH:
		إذا (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->مهالوېش(event->معلومات.delay * 1000,
					html_object_refresh, o);
		}

		اكسر;

	حالة CONTENT_پیغام_LINK:
		/* Don't care about favicons that aren't on top level content */
		اكسر;

	حالة CONTENT_پیغام_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->معلومات.jsthread) = عدم;
		اكسر;

	حالة CONTENT_پیغام_GETDIMS:
		*(event->معلومات.getdims.viewport_پلنوالی) =
				content_get_پلنوالی_2(&c->بنسټ);
		*(event->معلومات.getdims.viewport_لوړوالی) =
				content_get_لوړوالی_2(&c->بنسټ);
		اكسر;

	حالة CONTENT_پیغام_SCROLL:
		إذا (box->scroll_x != عدم)
			scrollbar_سېټ(box->scroll_x, event->معلومات.scroll.x0,
					false);
		إذا (box->scroll_y != عدم)
			scrollbar_سېټ(box->scroll_y, event->معلومات.scroll.y0,
					false);
		اكسر;

	حالة CONTENT_پیغام_DRAGSAVE:
	{
		اتحاد content_پیغام_معلومات پیغام_معلومات;
		إذا (event->معلومات.dragsave.content == عدم)
			پیغام_معلومات.dragsave.content = object;
		وإلا
			پیغام_معلومات.dragsave.content =
					event->معلومات.dragsave.content;

		content_خپرونه(&c->بنسټ, CONTENT_پیغام_DRAGSAVE, &پیغام_معلومات);
	}
		اكسر;

	حالة CONTENT_پیغام_SAVELINK:
	حالة CONTENT_پیغام_اشاره_ګر:
	حالة CONTENT_پیغام_SELECTMENU:
	حالة CONTENT_پیغام_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_خپرونه(&c->بنسټ, event->type, &event->معلومات);
		اكسر;

	حالة CONTENT_پیغام_CARET:
	{
		اتحاد html_focus_owner focus_owner;
		focus_owner.content = box;

		بدل (event->معلومات.caret.type) {
		حالة CONTENT_CARET_REMOVE:
		حالة CONTENT_CARET_HIDE:
			html_سېټ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, عدم);
			اكسر;
		حالة CONTENT_CARET_سېټ_POS:
			html_سېټ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->معلومات.caret.pos.x,
					event->معلومات.caret.pos.y,
					event->معلومات.caret.pos.لوړوالی,
					event->معلومات.caret.pos.clip);
			اكسر;
		}
	}
		اكسر;

	حالة CONTENT_پیغام_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اتحاد html_drag_owner drag_owner;
		drag_owner.content = box;

		بدل (event->معلومات.drag.type) {
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
		html_سېټ_drag_type(c, drag_type, drag_owner,
				event->معلومات.drag.rect);
	}
		اكسر;

	حالة CONTENT_پیغام_SELECTION:
	{
		html_selection_type sel_type;
		اتحاد html_selection_owner sel_owner;

		إذا (event->معلومات.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} وإلا {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_سېټ_selection(c, sel_type, sel_owner,
				event->معلومات.selection.read_only);
	}
		اكسر;

	افتراضي:
		اكسر;
	}

	إذا (c->بنسټ.حالت == CONTENT_حالت_READY &&
	    c->بنسټ.فعال == 0 &&
	    (event->type == CONTENT_پیغام_LOADING ||
	     event->type == CONTENT_پیغام_بشپړ ||
	     event->type == CONTENT_پیغام_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->بنسټ, false, c->بنسټ.شته_پلنوالی,
				c->بنسټ.شته_لوړوالی);
		content_سېټ_بشپړ(&c->بنسټ);
	} وإلا إذا (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_پیغام_بشپړ &&
		   box != عدم &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->بنسټ.حالت == CONTENT_حالت_READY ||
		    c->بنسټ.حالت == CONTENT_حالت_بشپړ)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		إذا (ms_now > c->بنسټ.reformat_وخت) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->بنسټ,
					  false,
					  c->بنسټ.شته_پلنوالی,
					  c->بنسټ.شته_لوړوالی);
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
	hlcache_ماشوم_context ماشوم;
	html_content *پاڼه;
	nserror error;

	أكد(object != عدم);
	أكد(object->box != عدم);

	c = (html_content *) object->پلار;

	ماشوم.charset = c->encoding;
	ماشوم.quirks = c->بنسټ.quirks;

	إذا (object->content != عدم) {
		/* remove existing object */
		إذا (content_get_حالت(object->content) != CONTENT_حالت_بشپړ) {
			c->بنسټ.فعال--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->بنسټ.فعال);
		}

		hlcache_handle_release(object->content);
		object->content = عدم;

		object->box->object = عدم;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->بنسټ), عدم,
			html_object_callback, object, &ماشوم,
			object->permitted_types,
			&object->content);

	إذا (error != NSERROR_OK)
		أرجع false;

	لكل (پاڼه = c; پاڼه != عدم; پاڼه = پاڼه->پاڼه) {
		پاڼه->بنسټ.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

		پاڼه->بنسټ.حالت = CONTENT_حالت_READY;
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

	content_invalidate_reuse_معلومات(object->content);

	إذا (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, بنية browser_کړکۍ *bw)
{
	بنية content_html_object *object, *بل;

	لكل (object = html->object_list; object != عدم; object = بل) {
		بل = object->بل;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		content_open(object->content,
			     bw,
			     &html->بنسټ,
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
	     object = object->بل) {
		إذا (object->content == عدم)
			استمر;

		بدل (content_get_حالت(object->content)) {
		حالة CONTENT_حالت_بشپړ:
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
				htmlc->بنسټ.فعال--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->بنسټ.فعال);
			}
			اكسر;

		}
	}

	أرجع NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	بنية content_html_object *object, *بل;

	لكل (object = html->object_list; object != عدم; object = بل) {
		بل = object->بل;

		إذا (object->content == عدم || object->box == عدم)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_NONE)
			استمر;

		إذا (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->مهالوېش(-1, html_object_refresh, object);
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
				guit->misc->مهالوېش(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->بل;
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
	hlcache_ماشوم_context ماشوم;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	إذا (c->aborted)
		أرجع true;

	ماشوم.charset = c->encoding;
	ماشوم.quirks = c->بنسټ.quirks;

	object = حافظه_صفر_ورکړه(1, حجم(بنية content_html_object));
	إذا (object == عدم) {
		أرجع false;
	}

	إذا (box == عدم) {
		object_callback = html_object_nobox_callback;
	} وإلا {
		object_callback = html_object_callback;
	}

	object->پلار = (بنية content *) c;
	object->بل = عدم;
	object->content = عدم;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->بنسټ),
					عدم,
					object_callback,
					object,
					&ماشوم,
					object->permitted_types,
					&object->content);
	إذا (error != NSERROR_OK) {
		حرر(object);
		أرجع error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->بل = c->object_list;
	c->object_list = object;

	c->عدد_objects++;
	إذا (box != عدم) {
		c->بنسټ.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);
	}

	أرجع true;
}
