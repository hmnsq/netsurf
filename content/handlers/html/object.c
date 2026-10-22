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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <nsutils/time.h>

#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "css/utils.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/interaction.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/object.h"

/* break reference loop */
స్థిర శూన్య html_object_refresh(శూన్య *p);

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
నిర్మాణం content_html_object *html_get_objects(hlcache_handle *h, గుర్తులేని పూర్ణాంకం *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);
	ప్రతిజ్ఞా(n != ఖాళీ);

	*n = c->అంకె_objects;

	తిరిగి c->object_list;
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

స్థిర శూన్య
html_object_failed(నిర్మాణం box *box, html_content *content, bool background)
{
	/* Nothing to do */
	తిరిగి;
}

/**
 * Update a box whose content has completed rendering.
 */

స్థిర శూన్య
html_object_పూర్తి(నిర్మాణం box *box,
		 hlcache_handle *object,
		 bool background)
{
	నిర్మాణం box *b;

	ఒకవేళ (background) {
		box->background = object;
		తిరిగి;
	}

	box->object = object;

	/* Normalise the box type, now it has been replaced. */
	ఎంపిక (box->type) {
	స్థితి BOX_పట్టిక:
		box->type = BOX_ఖండం;
		ఆపు;
	అప్రమేయ:
		/* TODO: Any other box types need mapping? */
		ఆపు;
	}

	ఒకవేళ (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		కొరకు (b = box; b; b = b->మాతృ)
			b->max_వెడల్పు = UNKNOWN_MAX_వెడల్పు;

		/* delete any clones of this box */
		అయితే (box->తదుపరి && (box->తదుపరి->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->తదుపరి = box->తదుపరి->తదుపరి;
		}
	}
}


/**
 * Callback for hlcache_handle_retrieve() for objects with no box.
 */
స్థిర nserror
html_object_nobox_callback(hlcache_handle *object,
			   స్థిరరాశి hlcache_event *event,
			   శూన్య *pw)
{
	నిర్మాణం content_html_object *chobject = pw;

	ఎంపిక (event->type) {
	స్థితి CONTENT_సందేశం_ERROR:
		hlcache_handle_release(object);

		chobject->content = ఖాళీ;
		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	తిరిగి NSERROR_OK;
}


/**
 * Callback for hlcache_handle_retrieve() for objects with a box.
 */
స్థిర nserror
html_object_callback(hlcache_handle *object,
		     స్థిరరాశి hlcache_event *event,
		     శూన్య *pw)
{
	నిర్మాణం content_html_object *o = pw;
	html_content *c = (html_content *) o->మాతృ;
	పూర్ణాంకం x, y;
	నిర్మాణం box *box;

	box = o->box;

	ఎంపిక (event->type) {
	స్థితి CONTENT_సందేశం_LOADING:
		ఒకవేళ (c->పునాది.స్థితి_2 != CONTENT_స్థితి_LOADING && c->bw != ఖాళీ)
			content_open(object,
					c->bw, &c->పునాది,
					box->object_params);
		ఆపు;

	స్థితి CONTENT_సందేశం_READY:
		ఒకవేళ (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు ?
							box->వెడల్పు : 0,
					box->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు ?
							box->ఎత్తు : 0);

			/* Adjust parent content for new object size */
			html_object_పూర్తి(box, object, o->background);
			ఒకవేళ (c->పునాది.స్థితి_2 == CONTENT_స్థితి_READY ||
					c->పునాది.స్థితి_2 == CONTENT_స్థితి_పూర్తి)
				content__reformat(&c->పునాది, false,
						c->పునాది.అందుబాటు_వెడల్పు,
						c->పునాది.అందుబాటు_ఎత్తు);
		}
		ఆపు;

	స్థితి CONTENT_సందేశం_పూర్తి:
		c->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

		html_object_పూర్తి(box, object, o->background);

		ఒకవేళ (c->పునాది.స్థితి_2 != CONTENT_స్థితి_LOADING &&
				box->flags & REPLACE_DIM) {
			సంఘం content_సందేశం_దత్తాంశం దత్తాంశం;

			ఒకవేళ (c->had_initial_layout == false) {
				ఆపు;
			}

			ఒకవేళ (!box_visible(box))
				ఆపు;

			box_coords(box, &x, &y);

			దత్తాంశం.redraw.x = x + box->padding[ఎడమ_2];
			దత్తాంశం.redraw.y = y + box->padding[శిఖరం_2];
			దత్తాంశం.redraw.వెడల్పు = box->వెడల్పు;
			దత్తాంశం.redraw.ఎత్తు = box->ఎత్తు;

			content_ప్రసారం(&c->పునాది, CONTENT_సందేశం_REDRAW, &దత్తాంశం);
		}
		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		hlcache_handle_release(object);

		o->content = ఖాళీ;

		c->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

		html_object_failed(box, c, o->background);

		ఆపు;

	స్థితి CONTENT_సందేశం_REDRAW:
		ఒకవేళ (c->పునాది.స్థితి_2 != CONTENT_స్థితి_LOADING) {
			సంఘం content_సందేశం_దత్తాంశం దత్తాంశం = event->దత్తాంశం;

			ఒకవేళ (c->had_initial_layout == false) {
				ఆపు;
			}

			ఒకవేళ (!box_visible(box))
				ఆపు;

			box_coords(box, &x, &y);

			ఒకవేళ (object == box->background) {
				/* Redraw request is for background */
				css_fixed hpos = 0, vpos = 0;
				css_unit hunit = CSS_UNIT_PX;
				css_unit vunit = CSS_UNIT_PX;
				పూర్ణాంకం వెడల్పు = box->padding[ఎడమ_2] + box->వెడల్పు +
						box->padding[కుడి_2];
				పూర్ణాంకం ఎత్తు = box->padding[శిఖరం_2] + box->ఎత్తు +
						box->padding[BOTTOM];
				పూర్ణాంకం t, h, l, w;

				/* Need to know background-position */
				css_computed_background_position(box->style,
						&hpos, &hunit, &vpos, &vunit);

				w = content_get_వెడల్పు(box->background);
				ఒకవేళ (hunit == CSS_UNIT_PCT) {
					l = (వెడల్పు - w) * hpos / INTTOFIX(100);
				} లేకపోతే {
					l = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							hpos, hunit));
				}

				h = content_get_ఎత్తు(box->background);
				ఒకవేళ (vunit == CSS_UNIT_PCT) {
					t = (ఎత్తు - h) * vpos / INTTOFIX(100);
				} లేకపోతే {
					t = FIXTOINT(css_unit_len2device_px(
							box->style,
							&c->unit_len_ctx,
							vpos, vunit));
				}

				/* Redraw area depends on background-repeat */
				ఎంపిక (css_computed_background_repeat(
						box->style)) {
				స్థితి CSS_BACKGROUND_REPEAT_REPEAT:
					దత్తాంశం.redraw.x = 0;
					దత్తాంశం.redraw.y = 0;
					దత్తాంశం.redraw.వెడల్పు = box->వెడల్పు;
					దత్తాంశం.redraw.ఎత్తు = box->ఎత్తు;
					ఆపు;

				స్థితి CSS_BACKGROUND_REPEAT_REPEAT_X:
					దత్తాంశం.redraw.x = 0;
					దత్తాంశం.redraw.y += t;
					దత్తాంశం.redraw.వెడల్పు = box->వెడల్పు;
					ఆపు;

				స్థితి CSS_BACKGROUND_REPEAT_REPEAT_Y:
					దత్తాంశం.redraw.x += l;
					దత్తాంశం.redraw.y = 0;
					దత్తాంశం.redraw.ఎత్తు = box->ఎత్తు;
					ఆపు;

				స్థితి CSS_BACKGROUND_REPEAT_NO_REPEAT:
					దత్తాంశం.redraw.x += l;
					దత్తాంశం.redraw.y += t;
					ఆపు;

				అప్రమేయ:
					ఆపు;
				}

				/* Add offset to box */
				దత్తాంశం.redraw.x += x;
				దత్తాంశం.redraw.y += y;

			} లేకపోతే {
				/* Non-background case */
				పూర్ణాంకం w = content_get_వెడల్పు(object);
				పూర్ణాంకం h = content_get_ఎత్తు(object);

				ఒకవేళ (w != 0 && box->వెడల్పు != w) {
					/* Not showing image at intrinsic
					 * width; need to scale the redraw
					 * request area. */
					దత్తాంశం.redraw.x = దత్తాంశం.redraw.x *
							box->వెడల్పు / w;
					దత్తాంశం.redraw.వెడల్పు =
							దత్తాంశం.redraw.వెడల్పు *
							box->వెడల్పు / w;
				}

				ఒకవేళ (h != 0 && box->ఎత్తు != w) {
					/* Not showing image at intrinsic
					 * height; need to scale the redraw
					 * request area. */
					దత్తాంశం.redraw.y = దత్తాంశం.redraw.y *
							box->ఎత్తు / h;
					దత్తాంశం.redraw.ఎత్తు =
							దత్తాంశం.redraw.ఎత్తు *
							box->ఎత్తు / h;
				}

				దత్తాంశం.redraw.x += x + box->padding[ఎడమ_2];
				దత్తాంశం.redraw.y += y + box->padding[శిఖరం_2];
			}

			content_ప్రసారం(&c->పునాది, CONTENT_సందేశం_REDRAW, &దత్తాంశం);
		}
		ఆపు;

	స్థితి CONTENT_సందేశం_REFRESH:
		ఒకవేళ (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			guit->misc->కాలపట్టిక(event->దత్తాంశం.delay * 1000,
					html_object_refresh, o);
		}

		ఆపు;

	స్థితి CONTENT_సందేశం_LINK:
		/* Don't care about favicons that aren't on top level content */
		ఆపు;

	స్థితి CONTENT_సందేశం_GETTHREAD:
		/* Objects don't have JS threads */
		*(event->దత్తాంశం.jsthread) = ఖాళీ;
		ఆపు;

	స్థితి CONTENT_సందేశం_GETDIMS:
		*(event->దత్తాంశం.getdims.viewport_వెడల్పు) =
				content_get_వెడల్పు_2(&c->పునాది);
		*(event->దత్తాంశం.getdims.viewport_ఎత్తు) =
				content_get_ఎత్తు_2(&c->పునాది);
		ఆపు;

	స్థితి CONTENT_సందేశం_SCROLL:
		ఒకవేళ (box->scroll_x != ఖాళీ)
			scrollbar_సమితి(box->scroll_x, event->దత్తాంశం.scroll.x0,
					false);
		ఒకవేళ (box->scroll_y != ఖాళీ)
			scrollbar_సమితి(box->scroll_y, event->దత్తాంశం.scroll.y0,
					false);
		ఆపు;

	స్థితి CONTENT_సందేశం_DRAGSAVE:
	{
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
		ఒకవేళ (event->దత్తాంశం.dragsave.content == ఖాళీ)
			సందేశం_దత్తాంశం.dragsave.content = object;
		లేకపోతే
			సందేశం_దత్తాంశం.dragsave.content =
					event->దత్తాంశం.dragsave.content;

		content_ప్రసారం(&c->పునాది, CONTENT_సందేశం_DRAGSAVE, &సందేశం_దత్తాంశం);
	}
		ఆపు;

	స్థితి CONTENT_సందేశం_SAVELINK:
	స్థితి CONTENT_సందేశం_సూచిక:
	స్థితి CONTENT_సందేశం_SELECTMENU:
	స్థితి CONTENT_సందేశం_GADGETCLICK:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_ప్రసారం(&c->పునాది, event->type, &event->దత్తాంశం);
		ఆపు;

	స్థితి CONTENT_సందేశం_CARET:
	{
		సంఘం html_focus_owner focus_owner;
		focus_owner.content = box;

		ఎంపిక (event->దత్తాంశం.caret.type) {
		స్థితి CONTENT_CARET_REMOVE:
		స్థితి CONTENT_CARET_HIDE:
			html_సమితి_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					true, 0, 0, 0, ఖాళీ);
			ఆపు;
		స్థితి CONTENT_CARET_సమితి_POS:
			html_సమితి_focus(c, HTML_FOCUS_CONTENT, focus_owner,
					false, event->దత్తాంశం.caret.pos.x,
					event->దత్తాంశం.caret.pos.y,
					event->దత్తాంశం.caret.pos.ఎత్తు,
					event->దత్తాంశం.caret.pos.clip);
			ఆపు;
		}
	}
		ఆపు;

	స్థితి CONTENT_సందేశం_DRAG:
	{
		html_drag_type drag_type = HTML_DRAG_NONE;
		సంఘం html_drag_owner drag_owner;
		drag_owner.content = box;

		ఎంపిక (event->దత్తాంశం.drag.type) {
		స్థితి CONTENT_DRAG_NONE:
			drag_type = HTML_DRAG_NONE;
			drag_owner.no_owner = true;
			ఆపు;
		స్థితి CONTENT_DRAG_SCROLL:
			drag_type = HTML_DRAG_CONTENT_SCROLL;
			ఆపు;
		స్థితి CONTENT_DRAG_SELECTION:
			drag_type = HTML_DRAG_CONTENT_SELECTION;
			ఆపు;
		}
		html_సమితి_drag_type(c, drag_type, drag_owner,
				event->దత్తాంశం.drag.rect);
	}
		ఆపు;

	స్థితి CONTENT_సందేశం_SELECTION:
	{
		html_selection_type sel_type;
		సంఘం html_selection_owner sel_owner;

		ఒకవేళ (event->దత్తాంశం.selection.selection) {
			sel_type = HTML_SELECTION_CONTENT;
			sel_owner.content = box;
		} లేకపోతే {
			sel_type = HTML_SELECTION_NONE;
			sel_owner.none = true;
		}
		html_సమితి_selection(c, sel_type, sel_owner,
				event->దత్తాంశం.selection.read_only);
	}
		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	ఒకవేళ (c->పునాది.స్థితి_2 == CONTENT_స్థితి_READY &&
	    c->పునాది.చురుకైన == 0 &&
	    (event->type == CONTENT_సందేశం_LOADING ||
	     event->type == CONTENT_సందేశం_పూర్తి ||
	     event->type == CONTENT_సందేశం_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->పునాది, false, c->పునాది.అందుబాటు_వెడల్పు,
				c->పునాది.అందుబాటు_ఎత్తు);
		content_సమితి_పూర్తి(&c->పునాది);
	} లేకపోతే ఒకవేళ (nsoption_bool(incremental_reflow) &&
		   event->type == CONTENT_సందేశం_పూర్తి &&
		   box != ఖాళీ &&
		   !(box->flags & REPLACE_DIM) &&
		   (c->పునాది.స్థితి_2 == CONTENT_స్థితి_READY ||
		    c->పునాది.స్థితి_2 == CONTENT_స్థితి_పూర్తి)) {
		/* 1) the configuration option to reflow pages while
		 *      objects are fetched is set
		 * 2) an object is newly fetched & converted,
		 * 3) the box's dimensions need to change due to being replaced
		 * 4) the object's parent HTML is ready for reformat,
		 */
		uint64_t ms_now;
		nsu_getmonotonic_ms(&ms_now);
		ఒకవేళ (ms_now > c->పునాది.reformat_సమయం) {
			/* The time since the previous reformat is
			 *  more than the configured minimum time
			 *  between reformats so reformat the page to
			 *  display newly fetched objects
			 */
			content__reformat(&c->పునాది,
					  false,
					  c->పునాది.అందుబాటు_వెడల్పు,
					  c->పునాది.అందుబాటు_ఎత్తు);
		}
	}

	తిరిగి NSERROR_OK;
}


/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */
స్థిర bool html_replace_object(నిర్మాణం content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_బిడ్డ_context బిడ్డ;
	html_content *పుట;
	nserror error;

	ప్రతిజ్ఞా(object != ఖాళీ);
	ప్రతిజ్ఞా(object->box != ఖాళీ);

	c = (html_content *) object->మాతృ;

	బిడ్డ.charset = c->encoding;
	బిడ్డ.quirks = c->పునాది.quirks;

	ఒకవేళ (object->content != ఖాళీ) {
		/* remove existing object */
		ఒకవేళ (content_get_స్థితి(object->content) != CONTENT_స్థితి_పూర్తి) {
			c->పునాది.చురుకైన--;
			NSLOG(netsurf, INFO, "%d fetches active",
			      c->పునాది.చురుకైన);
		}

		hlcache_handle_release(object->content);
		object->content = ఖాళీ;

		object->box->object = ఖాళీ;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->పునాది), ఖాళీ,
			html_object_callback, object, &బిడ్డ,
			object->permitted_types,
			&object->content);

	ఒకవేళ (error != NSERROR_OK)
		తిరిగి false;

	కొరకు (పుట = c; పుట != ఖాళీ; పుట = పుట->పుట) {
		పుట->పునాది.చురుకైన++;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

		పుట->పునాది.స్థితి_2 = CONTENT_స్థితి_READY;
	}

	తిరిగి true;
}

/**
 * schedule callback for object refresh
 */
స్థిర శూన్య html_object_refresh(శూన్య *p)
{
	నిర్మాణం content_html_object *object = p;
	nsurl *refresh_url;

	ప్రతిజ్ఞా(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	ఒకవేళ (refresh_url == ఖాళీ)
		తిరిగి;

	content_invalidate_reuse_దత్తాంశం(object->content);

	ఒకవేళ (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}


/* exported interface documented in html/object.h */
nserror html_object_open_objects(html_content *html, నిర్మాణం browser_కిటికీ *bw)
{
	నిర్మాణం content_html_object *object, *తదుపరి;

	కొరకు (object = html->object_list; object != ఖాళీ; object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		ఒకవేళ (object->content == ఖాళీ || object->box == ఖాళీ)
			కొనసాగు;

		ఒకవేళ (content_get_type(object->content) == CONTENT_NONE)
			కొనసాగు;

		content_open(object->content,
			     bw,
			     &html->పునాది,
			     object->box->object_params);
	}
	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_abort_objects(html_content *htmlc)
{
	నిర్మాణం content_html_object *object;

	కొరకు (object = htmlc->object_list;
	     object != ఖాళీ;
	     object = object->తదుపరి) {
		ఒకవేళ (object->content == ఖాళీ)
			కొనసాగు;

		ఎంపిక (content_get_స్థితి(object->content)) {
		స్థితి CONTENT_స్థితి_పూర్తి:
			/* already loaded: do nothing */
			ఆపు;

		స్థితి CONTENT_స్థితి_READY:
			hlcache_handle_abort(object->content);
			/* Active count will be updated when
			 * html_object_callback receives
			 * CONTENT_MSG_DONE from this object
			 */
			ఆపు;

		అప్రమేయ:
			hlcache_handle_abort(object->content);
			hlcache_handle_release(object->content);
			object->content = ఖాళీ;
			ఒకవేళ (object->box != ఖాళీ) {
				htmlc->పునాది.చురుకైన--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      htmlc->పునాది.చురుకైన);
			}
			ఆపు;

		}
	}

	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_close_objects(html_content *html)
{
	నిర్మాణం content_html_object *object, *తదుపరి;

	కొరకు (object = html->object_list; object != ఖాళీ; object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		ఒకవేళ (object->content == ఖాళీ || object->box == ఖాళీ)
			కొనసాగు;

		ఒకవేళ (content_get_type(object->content) == CONTENT_NONE)
			కొనసాగు;

		ఒకవేళ (content_get_type(object->content) == CONTENT_HTML) {
			guit->misc->కాలపట్టిక(-1, html_object_refresh, object);
		}

		content_close(object->content);
	}
	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/object.h */
nserror html_object_free_objects(html_content *html)
{
	అయితే (html->object_list != ఖాళీ) {
		నిర్మాణం content_html_object *victim = html->object_list;

		ఒకవేళ (victim->content != ఖాళీ) {
			NSLOG(netsurf, INFO, "object %p", victim->content);

			ఒకవేళ (content_get_type(victim->content) == CONTENT_HTML) {
				guit->misc->కాలపట్టిక(-1, html_object_refresh, victim);
			}
			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->తదుపరి;
		విడుదల(victim);
	}
	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/object.h */
bool
html_fetch_object(html_content *c,
		  nsurl *url,
		  నిర్మాణం box *box,
		  content_type permitted_types,
		  bool background)
{
	నిర్మాణం content_html_object *object;
	hlcache_handle_callback object_callback;
	hlcache_బిడ్డ_context బిడ్డ;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	ఒకవేళ (c->aborted)
		తిరిగి true;

	బిడ్డ.charset = c->encoding;
	బిడ్డ.quirks = c->పునాది.quirks;

	object = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content_html_object));
	ఒకవేళ (object == ఖాళీ) {
		తిరిగి false;
	}

	ఒకవేళ (box == ఖాళీ) {
		object_callback = html_object_nobox_callback;
	} లేకపోతే {
		object_callback = html_object_callback;
	}

	object->మాతృ = (నిర్మాణం content *) c;
	object->తదుపరి = ఖాళీ;
	object->content = ఖాళీ;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
					HLCACHE_RETRIEVE_SNIFF_TYPE,
					content_get_url(&c->పునాది),
					ఖాళీ,
					object_callback,
					object,
					&బిడ్డ,
					object->permitted_types,
					&object->content);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(object);
		తిరిగి error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->తదుపరి = c->object_list;
	c->object_list = object;

	c->అంకె_objects++;
	ఒకవేళ (box != ఖాళీ) {
		c->పునాది.చురుకైన++;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);
	}

	తిరిగి true;
}
