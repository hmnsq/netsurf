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

#شامل <assert.h>
#شامل <ctype.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdlib.h>
#شامل <nsutils/time.h>

#شامل "utils/corestrings.h"
#شامل "utils/config.h"
#شامل "utils/log.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/content.h"
#شامل "netsurf/misc.h"
#شامل "content/hlcache.h"
#شامل "css/utils.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/gui_internal.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/object.h"

/* break reference loop */
ساکن خالی html_object_refresh(خالی *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
ساخت content_html_object *html_get_objects(hlcache_handle *h, غیرعلامتی صحیح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);
	أكد(n != تہی);

	*n = c->عدد_objects;

	واپس c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

ساکن خالی
html_object_failed(ساخت box *box, html_content *content, bool background)
{
	/* Nothing to do */
	واپس;
}

/**
 * Update a box whose content has completed rendering.
 */

ساکن خالی
html_object_مکمل(ساخت box *box,
		 hlcache_handle *object,
		 bool background)
{
	ساخت box *b;

	اگر (background) {
		box->background = object;
		واپس;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	انتخاب (box->type) {
	حالت BOX_جدول:
		box->type = BOX_بلاک;
		توڑو;
	طےشدہ:
		/* TODO: Any other box types need mapping? */
		توڑو;
	}

	اگر (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		برائے (b = box; b; b = b->والد)
			b->max_چوڑائی = UNKNOWN_MAX_چوڑائی;

		/* delete any clones of this box */
		جب (box->اگلا && (box->اگلا->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->اگلا = box->اگلا->اگلا;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
ساکن nserror
html_object_nobox_callback(hlcache_handle *object,
			   ثابت hlcache_event *event,
			   خالی *pw)
{
	ساخت content_html_object *chobject = pw;

	انتخاب (event->type) {
	حالت CONTENT_پیغام_ERROR:
		hlcache_handle_release(object);

		chobject->content = تہی;
		توڑو;

	طےشدہ:
		توڑو;
	}

	واپس NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
ساکن nserror
html_object_callback(hlcache_handle *object,
		     ثابت hlcache_event *event,
		     خالی *pw)
{
	ساخت content_html_object *o = pw;
	html_content *c = (html_content *) o->والد;
	صحیح x, y;
	ساخت box *box;

	box = o->box;

	انتخاب (event->type) {
	حالت CONTENT_پیغام_LOADING:
		اگر (c->بنیاد.حالت_2 != CONTENT_حالت_LOADING && c->bw != تہی)
			content_open(object,
					c->bw, &c->بنیاد,
					box->object_params);
		توڑو;

	حالت CONTENT_پیغام_READY:
		اگر (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_چوڑائی != UNKNOWN_MAX_چوڑائی ?
							box->چوڑائی : 0,
					box->max_چوڑائی != UNKNOWN_MAX_چوڑائی ?
							box->اونچائی : 0);

			/* Adjust parent content for new object size */
			html_object_مکمل(box, object, o->background);
			اگر (c->بنیاد.حالت_2 == CONTENT_حالت_READY ||
					c->بنیاد.حالت_2 == CONTENT_حالت_مکمل)
				content__reformat(&c->بنیاد, false,
						c->بنیاد.دستیاب_چوڑائی,
						c->بنیاد.دستیاب_اونچائی);
		}
		توڑو;

	حالت CONTENT_پیغام_مکمل:
		c->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

		html_object_مکمل(box, object, o->background);

		اگر (c->بنیاد.حالت_2 != CONTENT_حالت_LOADING &&
				box->flags & REPLACE_DIM) {
			اتحاد content_پیغام_ڈیٹا ڈیٹا;

			اگر (c->had_initial_layout == false) {
				توڑو;
			}

			اگر (!box_visible(box))
				توڑو;

			box_coords(box, &x, &y);

			ڈیٹا.redraw.x = x + box->padding[بایاں_2];
			ڈیٹا.redraw.y = y + box->padding[چوٹی_2];
			ڈیٹا.redraw.چوڑائی = box->چوڑائی;
			ڈیٹا.redraw.اونچائی = box->اونچائی;

			content_نشر(&c->بنیاد, CONTENT_پیغام_REDRAW, &ڈیٹا);
		}
		توڑو;

	حالت CONTENT_پیغام_ERROR:
		hlcache_handle_release(object);

		o->content = تہی;

		c->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

		html_object_failed(box, c, o->background);

		توڑو;

	حالت CONTENT_پیغام_REDRAW:
		اگر (c->بنیاد.حالت_2 != CONTENT_حالت_LOADING) {
			اتحاد content_پیغام_ڈیٹا ڈیٹا = event->ڈیٹا;

			اگر (c->had_initial_layout == false) {
				توڑو;
			}

			اگر (!box_visible(box))
				توڑو;

			box_coords(box, &x, &y);

			اگر (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				صحیح چوڑائی = box->padding[بایاں_2] + box->چوڑائی +
						box->padding[دایاں_2];
				صحیح اونچائی = box->padding[چوٹی_2] + box->اونچائی +
						box->padding[BOTTOM];
				صحیح t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_چوڑائی(box->background);
				اگر (hunit == CSS_UNIT_PCT) {
					l = (چوڑائی - w) * hpos / INTTOFIX(100);
				} ورنہ {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_اونچائی(box->background);
				اگر (vunit == CSS_UNIT_PCT) {
					t = (اونچائی - h) * vpos / INTTOFIX(100);
				} ورنہ {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				انتخاب (css_computed_background_repeat(
						box->style)) {
				حالت CSS_BACKGROUND_REPEAT_REPEAT:
					ڈیٹا.redraw.x = 0;
					ڈیٹا.redraw.y = 0;
					ڈیٹا.redraw.چوڑائی = box->چوڑائی;
					ڈیٹا.redraw.اونچائی = box->اونچائی;
					توڑو;

				حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
					ڈیٹا.redraw.x = 0;
					ڈیٹا.redraw.y += t;
					ڈیٹا.redraw.چوڑائی = box->چوڑائی;
					توڑو;

				حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
					ڈیٹا.redraw.x += l;
					ڈیٹا.redraw.y = 0;
					ڈیٹا.redraw.اونچائی = box->اونچائی;
					توڑو;

				حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
					ڈیٹا.redraw.x += l;
					ڈیٹا.redraw.y += t;
					توڑو;

				طےشدہ:
					توڑو;
				}

				/* Add offset to box */
				ڈیٹا.redraw.x += x;
				ڈیٹا.redraw.y += y;

			} ورنہ {
				/* Non-background case */
				صحیح w = content_get_چوڑائی(object);
				صحیح h = content_get_اونچائی(object);

				اگر (w != 0 && box->چوڑائی != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					ڈیٹا.redraw.x = ڈیٹا.redraw.x *
							box->چوڑائی / w;
					ڈیٹا.redraw.چوڑائی =
							ڈیٹا.redraw.چوڑائی *
							box->چوڑائی / w;
				}

				اگر (h != 0 && box->اونچائی != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					ڈیٹا.redraw.y = ڈیٹا.redraw.y *
							box->اونچائی / h;
					ڈیٹا.redraw.اونچائی =
							ڈیٹا.redraw.اونچائی *
							box->اونچائی / h;
				}

				ڈیٹا.redraw.x += x + box->padding[بایاں_2];
				ڈیٹا.redraw.y += y + box->padding[چوٹی_2];
			}

			content_نشر(&c->بنیاد, CONTENT_پیغام_REDRAW, &ڈیٹا);
		}
		توڑو;

	حالت CONTENT_پیغام_REFRESH:
		اگر (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->جدول_وقت(event->ڈیٹا.delay * 1000,
					html_object_refresh, o);
		}

		توڑو;

	حالت CONTENT_پیغام_LINK:
		/* Don't care about favicons that aren't on top level content */
		توڑو;

	حالت CONTENT_پیغام_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->ڈیٹا.jsthread) = تہی;
		توڑو;

	حالت CONTENT_پیغام_GETDIMS:
		*(event->ڈیٹا.getdims.viewport_چوڑائی) =
				content_get_چوڑائی_2(&c->بنیاد);
		*(event->ڈیٹا.getdims.viewport_اونچائی) =
				content_get_اونچائی_2(&c->بنیاد);
		توڑو;

	حالت CONTENT_پیغام_SCROLL:
		اگر (box->scroll_x != تہی)
			scrollbar_سیٹ(box->scroll_x, event->ڈیٹا.scroll.x0,
					false);
		اگر (box->scroll_y != تہی)
			scrollbar_سیٹ(box->scroll_y, event->ڈیٹا.scroll.y0,
					false);
		توڑو;

	حالت CONTENT_پیغام_DRAGSAVE:
	{
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
		اگر (event->ڈیٹا.dragsave.content == تہی)
			پیغام_ڈیٹا.dragsave.content = object;
		ورنہ
			پیغام_ڈیٹا.dragsave.content =
					event->ڈیٹا.dragsave.content;

		content_نشر(&c->بنیاد, CONTENT_پیغام_DRAGSAVE, &پیغام_ڈیٹا);
	}
		توڑو;

	حالت CONTENT_پیغام_SAVELINK:
	حالت CONTENT_پیغام_اشارہ_گر:
	حالت CONTENT_پیغام_SELECTMENU:
	حالت CONTENT_پیغام_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_نشر(&c->بنیاد, event->type, &event->ڈیٹا);
		توڑو;

	حالت CONTENT_پیغام_CARET:
	{
		اتحاد html_focus_owner focus_owner;
		focus_owner.content = box;

		انتخاب (event->ڈیٹا.caret.type) {
		حالت CONTENT_CARET_REMOVE:
		حالت CONTENT_CARET_HIDE:
			html_سیٹ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, تہی);
			توڑو;
		حالت CONTENT_CARET_سیٹ_POS:
			html_سیٹ_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->ڈیٹا.caret.pos.x,
					event->ڈیٹا.caret.pos.y,
					event->ڈیٹا.caret.pos.اونچائی,
					event->ڈیٹا.caret.pos.clip);
			توڑو;
		}
	}
		توڑو;

	حالت CONTENT_پیغام_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		اتحاد html_drag_owner drag_owner;
		drag_owner.content = box;

		انتخاب (event->ڈیٹا.drag.type) {
		حالت CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			توڑو;
		حالت CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			توڑو;
		حالت CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			توڑو;
		}
		html_سیٹ_drag_type(c, drag_type, drag_owner,
				event->ڈیٹا.drag.rect);
	}
		توڑو;

	حالت CONTENT_پیغام_SELECTION:
	{
		html_selection_type sel_type;
		اتحاد html_selection_owner sel_owner;

		اگر (event->ڈیٹا.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} ورنہ {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_سیٹ_selection(c, sel_type, sel_owner,
				event->ڈیٹا.selection.read_only);
	}
		توڑو;

	طےشدہ:
		توڑو;
	}

	اگر (c->بنیاد.حالت_2 == CONTENT_حالت_READY &&
	    c->بنیاد.فعال == 0 &&
	    (event->type == CONTENT_پیغام_LOADING ||
	     event->type == CONTENT_پیغام_مکمل ||
	     event->type == CONTENT_پیغام_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->بنیاد, false, c->بنیاد.دستیاب_چوڑائی,
				c->بنیاد.دستیاب_اونچائی);
		content_سیٹ_مکمل(&c->بنیاد);
	} ورنہ اگر (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_پیغام_مکمل &&
		   box != تہی &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->بنیاد.حالت_2 == CONTENT_حالت_READY ||
		    c->بنیاد.حالت_2 == CONTENT_حالت_مکمل)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		اگر (ms_now > c->بنیاد.reformat_وقت) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->بنیاد,
					  false,
					  c->بنیاد.دستیاب_چوڑائی,
					  c->بنیاد.دستیاب_اونچائی);
		}
	}

	واپس NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
ساکن bool html_replace_object(ساخت content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_بچہ_context بچہ;
	html_content *صفحہ;
	nserror error;

	أكد(object != تہی);
	أكد(object->box != تہی);

	c = (html_content *) object->والد;

	بچہ.charset = c->encoding;
	بچہ.quirks = c->بنیاد.quirks;

	اگر (object->content != تہی) {
		/* remove existing object */
		اگر (content_get_حالت(object->content) != CONTENT_حالت_مکمل) {
			c->بنیاد.فعال--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->بنیاد.فعال);
		}

		hlcache_handle_release(object->content);
		object->content = تہی;

		object->box->object = تہی;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->بنیاد), تہی,
			html_object_callback, object, &بچہ,
			object->permitted_types,
			&object->content);

	اگر (error != NSERROR_OK)
		واپس false;

	برائے (صفحہ = c; صفحہ != تہی; صفحہ = صفحہ->صفحہ) {
		صفحہ->بنیاد.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

		صفحہ->بنیاد.حالت_2 = CONTENT_حالت_READY;
	}

	واپس true;
}

/**
 * schedule callback for object refresh
 */
ساکن خالی html_object_refresh(خالی *p)
{
	ساخت content_html_object *object = p;
	nsurl *refresh_url;

	أكد(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	اگر (refresh_url == تہی)
		واپس;

	content_invalidate_reuse_ڈیٹا(object->content);

	اگر (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, ساخت browser_کھڑکی *bw)
{
	ساخت content_html_object *object, *اگلا;

	برائے (object = html->object_list; object != تہی; object = اگلا) {
		اگلا = object->اگلا;

		اگر (object->content == تہی || object->box == تہی)
			جاری;

		اگر (content_get_type(object->content) == CONTENT_NONE)
			جاری;

		content_open(object->content,
			     bw,
			     &html->بنیاد,
			     object->box->object_params);
	}
	واپس NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	ساخت content_html_object *object;

	برائے (object = htmlc->object_list;
	     object != تہی;
	     object = object->اگلا) {
		اگر (object->content == تہی)
			جاری;

		انتخاب (content_get_حالت(object->content)) {
		حالت CONTENT_حالت_مکمل:
			/* already loaded: do nothing */
			توڑو;

		حالت CONTENT_حالت_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			توڑو;

		طےشدہ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = تہی;
			اگر (object->box != تہی) {
				htmlc->بنیاد.فعال--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->بنیاد.فعال);
			}
			توڑو;

		}
	}

	واپس NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	ساخت content_html_object *object, *اگلا;

	برائے (object = html->object_list; object != تہی; object = اگلا) {
		اگلا = object->اگلا;

		اگر (object->content == تہی || object->box == تہی)
			جاری;

		اگر (content_get_type(object->content) == CONTENT_NONE)
			جاری;

		اگر (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->جدول_وقت(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	واپس NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	جب (html->object_list != تہی) {
		ساخت content_html_object *victim = html->object_list;

		اگر (victim->content != تہی) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			اگر (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->جدول_وقت(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->اگلا;
		آزاد(victim);
	}
	واپس NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  ساخت box *box,
		  content_type permitted_types,
		  bool background)
{
	ساخت content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_بچہ_context بچہ;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	اگر (c->aborted)
		واپس true;

	بچہ.charset = c->encoding;
	بچہ.quirks = c->بنیاد.quirks;

	object = صفر_حافظہ_دو(1, حجم(ساخت content_html_object));
	اگر (object == تہی) {
		واپس false;
	}

	اگر (box == تہی) {
		object_callback = html_object_nobox_callback;
	} ورنہ {
		object_callback = html_object_callback;
	}

	object->والد = (ساخت content *) c;
	object->اگلا = تہی;
	object->content = تہی;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->بنیاد),
					تہی,
					object_callback,
					object,
					&بچہ,
					object->permitted_types,
					&object->content);
	اگر (error != NSERROR_OK) {
		آزاد(object);
		واپس error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->اگلا = c->object_list;
	c->object_list = object;

	c->عدد_objects++;
	اگر (box != تہی) {
		c->بنیاد.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);
	}

	واپس true;
}
