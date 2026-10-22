/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
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
 * implementation of user interaction with a CONTENT_HTML.
 */

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/mouse.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "netsurf/layout.h"
#అన్తర్భావ "netsurf/keypress.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "content/textsearch.h"
#అన్తర్భావ "desktop/browser_history.h"
#అన్తర్భావ "desktop/frames.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/selection.h"
#అన్తర్భావ "desktop/textarea.h"
#అన్తర్భావ "javascript/js.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_textarea.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/imagemap.h"
#అన్తర్భావ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

స్థిర browser_సూచిక_shape get_సూచిక_shape(నిర్మాణం box *box, bool imagemap)
{
	browser_సూచిక_shape సూచిక;
	css_computed_style *style;
	గణన css_cursor_e cursor;
	lwc_string **cursor_uris;

	ఒకవేళ (box->type == BOX_FLOAT_ఎడమ || box->type == BOX_FLOAT_కుడి)
		style = box->children->style;
	లేకపోతే
		style = box->style;

	ఒకవేళ (style == ఖాళీ)
		తిరిగి BROWSER_సూచిక_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ఎంపిక (cursor) {
	స్థితి CSS_CURSOR_AUTO:
		ఒకవేళ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			సూచిక = BROWSER_సూచిక_బిందువు;
		} లేకపోతే ఒకవేళ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			సూచిక = BROWSER_సూచిక_CARET;
		} లేకపోతే {
			/* html content doesn't mind */
			సూచిక = BROWSER_సూచిక_AUTO;
		}
		ఆపు;
	స్థితి CSS_CURSOR_CROSSHAIR:
		సూచిక = BROWSER_సూచిక_CROSS;
		ఆపు;
	స్థితి CSS_CURSOR_సూచిక:
		సూచిక = BROWSER_సూచిక_బిందువు;
		ఆపు;
	స్థితి CSS_CURSOR_MOVE:
		సూచిక = BROWSER_సూచిక_MOVE;
		ఆపు;
	స్థితి CSS_CURSOR_E_RESIZE:
		సూచిక = BROWSER_సూచిక_కుడి;
		ఆపు;
	స్థితి CSS_CURSOR_W_RESIZE:
		సూచిక = BROWSER_సూచిక_ఎడమ;
		ఆపు;
	స్థితి CSS_CURSOR_N_RESIZE:
		సూచిక = BROWSER_సూచిక_పైకి;
		ఆపు;
	స్థితి CSS_CURSOR_S_RESIZE:
		సూచిక = BROWSER_సూచిక_కిందికి;
		ఆపు;
	స్థితి CSS_CURSOR_NE_RESIZE:
		సూచిక = BROWSER_సూచిక_RU;
		ఆపు;
	స్థితి CSS_CURSOR_SW_RESIZE:
		సూచిక = BROWSER_సూచిక_LD;
		ఆపు;
	స్థితి CSS_CURSOR_SE_RESIZE:
		సూచిక = BROWSER_సూచిక_RD;
		ఆపు;
	స్థితి CSS_CURSOR_NW_RESIZE:
		సూచిక = BROWSER_సూచిక_LU;
		ఆపు;
	స్థితి CSS_CURSOR_పాఠ్యం:
		సూచిక = BROWSER_సూచిక_CARET;
		ఆపు;
	స్థితి CSS_CURSOR_WAIT:
		సూచిక = BROWSER_సూచిక_WAIT;
		ఆపు;
	స్థితి CSS_CURSOR_పురోగతి:
		సూచిక = BROWSER_సూచిక_పురోగతి;
		ఆపు;
	స్థితి CSS_CURSOR_HELP:
		సూచిక = BROWSER_సూచిక_HELP;
		ఆపు;
	అప్రమేయ:
		సూచిక = BROWSER_సూచిక_DEFAULT;
		ఆపు;
	}

	తిరిగి సూచిక;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

స్థిర శూన్య html_box_drag_ప్రారంభం(నిర్మాణం box *box, పూర్ణాంకం x, పూర్ణాంకం y)
{
	పూర్ణాంకం box_x, box_y;
	పూర్ణాంకం scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ఒకవేళ (box->scroll_x != ఖాళీ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[శిఖరం_2] +
				box->ఎత్తు + box->padding[BOTTOM] -
				SCROLLBAR_వెడల్పు);
		scrollbar_ప్రారంభం_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} లేకపోతే ఒకవేళ (box->scroll_y != ఖాళీ) {
		scroll_mouse_x = x - (box_x + box->padding[ఎడమ_2] +
				box->వెడల్పు + box->padding[కుడి_2] -
				SCROLLBAR_వెడల్పు);
		scroll_mouse_y = y - box_y;

		scrollbar_ప్రారంభం_content_drag(box->scroll_y,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param html   html content
 * \param mouse  state of mouse buttons and modifier keys
 * \param x	 coordinate of mouse
 * \param y	 coordinate of mouse
 * \param dir    Direction of drag
 */
స్థిర size_t html_selection_drag_చివర(నిర్మాణం html_content *html,
		browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం dir)
{
	పూర్ణాంకం pixel_స్థానభ్రంశం;
	నిర్మాణం box *box;
	పూర్ణాంకం dx, dy;
	size_t idx = 0;

	box = box_pick_పాఠ్యం_box(html, x, y, dir, &dx, &dy);
	ఒకవేళ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->పాఠ్యం, box->నిడివి,
				       dx, &idx, &pixel_స్థానభ్రంశం);

		idx += box->byte_స్థానభ్రంశం;
	}

	తిరిగి idx;
}


/**
 * Helper for file gadgets to store their filename.
 *
 * Stores the filename unencoded on the dom node associated with the
 * gadget.
 *
 * \todo Get rid of this crap eventually
 *
 * \param operation DOM operation
 * \param key DOM node key being considerd
 * \param _data The data assocated with the key
 * \param src The source DOM node.
 * \param dst The destination DOM node.
 */
స్థిర శూన్య
html_image_coords_dom_user_దత్తాంశం_handler(dom_కణుపు_operation operation,
					 dom_string *కీలకం,
					 శూన్య *దత్తాంశం_2,
					 నిర్మాణం dom_కణుపు *src,
					 నిర్మాణం dom_కణుపు *dst)
{
	నిర్మాణం image_ఆదానం_coords *oldcoords, *coords = దత్తాంశం_2, *newcoords;

	ఒకవేళ (!dom_string_isequal(corestring_dom_ns_కీలకం_image_coords_కణుపు_దత్తాంశం,
				కీలకం) || coords == ఖాళీ) {
		తిరిగి;
	}

	ఎంపిక (operation) {
	స్థితి DOM_కణుపు_CLONED:
		newcoords = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*newcoords));
		ఒకవేళ (newcoords != ఖాళీ) {
			*newcoords = *coords;
			ఒకవేళ (dom_కణుపు_సమితి_user_దత్తాంశం(dst,
				 corestring_dom_ns_కీలకం_image_coords_కణుపు_దత్తాంశం,
				 newcoords,
				 html_image_coords_dom_user_దత్తాంశం_handler,
				 &oldcoords) == DOM_NO_ERR) {
				విడుదల(oldcoords);
			}
		}
		ఆపు;

	స్థితి DOM_కణుపు_DELETED:
		విడుదల(coords);
		ఆపు;

	స్థితి DOM_కణుపు_RENAMED:
	స్థితి DOM_కణుపు_IMPORTED:
	స్థితి DOM_కణుపు_ADOPTED:
		ఆపు;

	అప్రమేయ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ప్రతిజ్ఞా(0);
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param  scrollbar  scrollbar widget
 * \param  mouse   state of mouse buttons and modifier keys
 * \param  x	   coordinate of mouse
 * \param  y	   coordinate of mouse
 */
స్థిర శూన్య
html_అధికప్రవాహం_scroll_drag_చివర(నిర్మాణం scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      పూర్ణాంకం x, పూర్ణాంకం y)
{
	పూర్ణాంకం scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	నిర్మాణం html_scrollbar_దత్తాంశం *దత్తాంశం = scrollbar_get_దత్తాంశం(scrollbar);
	నిర్మాణం box *box;

	box = దత్తాంశం->box;
	box_coords(box, &box_x, &box_y);

	ఒకవేళ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[శిఖరం_2] +
				box->ఎత్తు + box->padding[BOTTOM] -
				SCROLLBAR_వెడల్పు);
		scrollbar_mouse_drag_చివర(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} లేకపోతే {
		scroll_mouse_x = x - (box_x + box->padding[ఎడమ_2] +
				box->వెడల్పు + box->padding[కుడి_2] -
				SCROLLBAR_వెడల్పు);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_చివర(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
స్థిర nserror
mouse_action_select_menu(html_content *html,
			 నిర్మాణం browser_కిటికీ *bw,
			 browser_mouse_state mouse,
			 పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం box *box;
	పూర్ణాంకం box_x = 0;
	పూర్ణాంకం box_y = 0;
	స్థిరరాశి అక్షరం *స్థితి_2;
	పూర్ణాంకం వెడల్పు, ఎత్తు;
	నిర్మాణం hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ప్రతిజ్ఞా(html->visible_select_menu != ఖాళీ);

	bw_drag_type = browser_కిటికీ_get_drag_type(bw);
	ఒకవేళ (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_చివర(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ఎడమ_2].వెడల్పు;
	box_y += box->ఎత్తు + box->border[BOTTOM].వెడల్పు +
		box->padding[BOTTOM] + box->padding[శిఖరం_2];

	స్థితి_2 = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ఒకవేళ (స్థితి_2 != ఖాళీ) {
		/* set status if menu still open */
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
		సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = స్థితి_2;
		content_ప్రసారం((నిర్మాణం content *)html,
				  CONTENT_సందేశం_స్థితి,
				  &సందేశం_దత్తాంశం);
		తిరిగి NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &వెడల్పు, &ఎత్తు);

	html->visible_select_menu = ఖాళీ;

	bw_content = browser_కిటికీ_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       వెడల్పు,
			       ఎత్తు);
	తిరిగి NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
స్థిర nserror
mouse_action_drag_selection(html_content *html,
			    నిర్మాణం browser_కిటికీ *bw,
			    browser_mouse_state mouse,
			    పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం box *box;
	పూర్ణాంకం dir = -1;
	పూర్ణాంకం dx, dy;
	size_t idx;
	సంఘం html_drag_owner drag_owner;
	పూర్ణాంకం pixel_స్థానభ్రంశం;
	plot_font_style_t fstyle;

	ఒకవేళ (!mouse) {
		/* End of selection drag */
		ఒకవేళ (selection_dragging_ప్రారంభం(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_చివర(html, mouse, x, y, dir);

		ఒకవేళ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_సమితి_drag_type(html, HTML_DRAG_NONE, drag_owner, ఖాళీ);

		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (selection_dragging_ప్రారంభం(html->sel)) {
		dir = 1;
	}

	box = box_pick_పాఠ్యం_box(html, x, y, dir, &dx, &dy);
	ఒకవేళ (box != ఖాళీ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->పాఠ్యం,
				       box->నిడివి,
				       dx,
				       &idx,
				       &pixel_స్థానభ్రంశం);

		selection_track(html->sel, mouse, box->byte_స్థానభ్రంశం + idx);
	}
	తిరిగి NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
స్థిర nserror
mouse_action_drag_scrollbar(html_content *html,
			    నిర్మాణం browser_కిటికీ *bw,
			    browser_mouse_state mouse,
			    పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం scrollbar *scr;
	నిర్మాణం html_scrollbar_దత్తాంశం *దత్తాంశం;
	నిర్మాణం box *box;
	పూర్ణాంకం box_x = 0;
	పూర్ణాంకం box_y = 0;
	స్థిరరాశి అక్షరం *స్థితి_2;
	పూర్ణాంకం scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_స్థితి scrollbar_స్థితి;

	scr = html->drag_owner.scrollbar;

	ఒకవేళ (!mouse) {
		/* drag end: scrollbar */
		html_అధికప్రవాహం_scroll_drag_చివర(scr, mouse, x, y);
	}

	దత్తాంశం = scrollbar_get_దత్తాంశం(scr);

	box = దత్తాంశం->box;

	box_coords(box, &box_x, &box_y);

	ఒకవేళ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[శిఖరం_2] +
				      box->ఎత్తు + box->padding[BOTTOM] -
				      SCROLLBAR_వెడల్పు);
		scrollbar_స్థితి = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} లేకపోతే {
		scroll_mouse_x = x - (box_x + box->padding[ఎడమ_2] +
				      box->వెడల్పు + box->padding[కుడి_2] -
				      SCROLLBAR_వెడల్పు);
		scroll_mouse_y = y - box_y;

		scrollbar_స్థితి = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	స్థితి_2 = scrollbar_mouse_స్థితి_గమ్యం_సందేశం(scrollbar_స్థితి);

	ఒకవేళ (స్థితి_2 != ఖాళీ) {
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

		సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = స్థితి_2;
		content_ప్రసారం((నిర్మాణం content *)html,
				  CONTENT_సందేశం_స్థితి,
				  &సందేశం_దత్తాంశం);
	}

	తిరిగి NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
స్థిర nserror
mouse_action_drag_textarea(html_content *html,
			    నిర్మాణం browser_కిటికీ *bw,
			    browser_mouse_state mouse,
			    పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం box *box;
	పూర్ణాంకం box_x = 0;
	పూర్ణాంకం box_y = 0;

	box = html->drag_owner.textarea;

	ప్రతిజ్ఞా(box->gadget != ఖాళీ);
	ప్రతిజ్ఞా(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->దత్తాంశం.పాఠ్యం.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	తిరిగి NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
స్థిర nserror
mouse_action_drag_content(html_content *html,
			  నిర్మాణం browser_కిటికీ *bw,
			  browser_mouse_state mouse,
			  పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం box *box;
	పూర్ణాంకం box_x = 0;
	పూర్ణాంకం box_y = 0;

	box = html->drag_owner.content;
	ప్రతిజ్ఞా(box->object != ఖాళీ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	తిరిగి NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
నిర్మాణం mouse_action_state {
	నిర్మాణం {
		స్థిరరాశి అక్షరం *స్థితి_2; /**< status text */
		browser_సూచిక_shape సూచిక; /**< pointer shape */
		గణన {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_వెనుక, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} ఫలితం;

	/** dom node */
	నిర్మాణం dom_కణుపు *కణుపు;

	/** html object */
	నిర్మాణం {
		నిర్మాణం box *box;
		పూర్ణాంకం pos_x;
		పూర్ణాంకం pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	నిర్మాణం browser_కిటికీ *iframe;

	/** link either from href or imagemap */
	నిర్మాణం {
		నిర్మాణం box *box;
		nsurl *url;
		స్థిరరాశి అక్షరం *లక్ష్యం;
		bool is_imagemap;
	} link;

	/** gadget */
	నిర్మాణం {
		నిర్మాణం form_control *control;
		నిర్మాణం box *box;
		పూర్ణాంకం box_x;
		పూర్ణాంకం box_y;
		స్థిరరాశి అక్షరం *లక్ష్యం;
	} gadget;

	/** title */
	స్థిరరాశి అక్షరం *title;

	/** candidate box for drag operation */
	నిర్మాణం box *drag_candidate;

	/** scrollbar */
	నిర్మాణం {
		నిర్మాణం scrollbar *bar;
		పూర్ణాంకం mouse_x;
		పూర్ణాంకం mouse_y;
	} scroll;

	/** text in box */
	నిర్మాణం {
		నిర్మాణం box *box;
		పూర్ణాంకం box_x;
	} పాఠ్యం;
};


/**
 * iterate the box tree for deepest node at coordinates
 *
 * extracts mouse action node information by descending through
 *  visible boxes setting more specific values for:
 *
 * box - deepest box at point
 * html_object_box - html object
 * html_object_pos_x - html object
 * html_object_pos_y - html object
 * object - non html object
 * iframe - iframe
 * url - href or imagemap
 * target - href or imagemap or gadget
 * url_box - href or imagemap
 * imagemap - imagemap
 * gadget - gadget
 * gadget_box - gadget
 * gadget_box_x - gadget
 * gadget_box_y - gadget
 * title - title
 * pointer
 *
 * drag_candidate - first box with scroll
 * padding_left - box with scroll
 * padding_right
 * padding_top
 * padding_bottom
 * scrollbar - inside padding box stops decent
 * scroll_mouse_x - inside padding box stops decent
 * scroll_mouse_y - inside padding box stops decent
 *
 * text_box - text box
 * text_box_x - text_box
 */
స్థిర nserror
get_mouse_action_కణుపు(html_content *html,
		      పూర్ణాంకం x, పూర్ణాంకం y,
		      నిర్మాణం mouse_action_state *man)
{
	నిర్మాణం box *box;
	పూర్ణాంకం box_x = 0;
	పూర్ణాంకం box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, పరిమాణం(నిర్మాణం mouse_action_state));
	man->కణుపు = html->layout->కణుపు; /* Default dom node to the <HTML> */
	man->ఫలితం.సూచిక = BROWSER_సూచిక_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ఎడమ_2];
	box_y = box->margin[శిఖరం_2];

	చేయి {
		/* skip hidden boxes */
		ఒకవేళ ((box->style != ఖాళీ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			వెళ్ళు తదుపరి_box;
		}

		ఒకవేళ (box->కణుపు != ఖాళీ) {
			man->కణుపు = box->కణుపు;
		}

		ఒకవేళ (box->object) {
			ఒకవేళ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} లేకపోతే {
				man->object = box->object;
			}
		}

		ఒకవేళ (box->iframe) {
			man->iframe = box->iframe;
		}

		ఒకవేళ (box->href) {
			man->link.url = box->href;
			man->link.లక్ష్యం = box->లక్ష్యం;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ఒకవేళ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.లక్ష్యం);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ఒకవేళ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ఒకవేళ (box->gadget->form) {
				man->gadget.లక్ష్యం = box->gadget->form->లక్ష్యం;
			}
		}

		ఒకవేళ (box->title) {
			man->title = box->title;
		}

		man->ఫలితం.సూచిక = get_సూచిక_shape(box, false);

		ఒకవేళ ((box->scroll_x != ఖాళీ) ||
		    (box->scroll_y != ఖాళీ)) {
			పూర్ణాంకం padding_ఎడమ;
			పూర్ణాంకం padding_కుడి;
			పూర్ణాంకం padding_శిఖరం;
			పూర్ణాంకం padding_bottom;

			ఒకవేళ (man->drag_candidate == ఖాళీ) {
				man->drag_candidate = box;
			}

			padding_ఎడమ = box_x +
					scrollbar_get_స్థానభ్రంశం(box->scroll_x);
			padding_కుడి = padding_ఎడమ + box->padding[ఎడమ_2] +
					box->వెడల్పు + box->padding[కుడి_2];
			padding_శిఖరం = box_y +
					scrollbar_get_స్థానభ్రంశం(box->scroll_y);
			padding_bottom = padding_శిఖరం + box->padding[శిఖరం_2] +
					box->ఎత్తు + box->padding[BOTTOM];

			ఒకవేళ ((x > padding_ఎడమ) &&
			    (x < padding_కుడి) &&
			    (y > padding_శిఖరం) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ఒకవేళ ((box->scroll_y != ఖాళీ) &&
				    (x > (padding_కుడి - SCROLLBAR_వెడల్పు))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_కుడి - SCROLLBAR_వెడల్పు);
					man->scroll.mouse_y = y - padding_శిఖరం;
					ఆపు;

				} లేకపోతే ఒకవేళ ((box->scroll_x != ఖాళీ) &&
					   (y > (padding_bottom -
							SCROLLBAR_వెడల్పు))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ఎడమ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_వెడల్పు);
					ఆపు;
				}
			}
		}

		ఒకవేళ (box->పాఠ్యం && !box->object) {
			man->పాఠ్యం.box = box;
			man->పాఠ్యం.box_x = box_x;
		}

	తదుపరి_box:
		/* iterate to next box */
		box = box_at_బిందువు(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} అయితే (box != ఖాళీ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ప్రతిజ్ఞా(man->కణుపు != ఖాళీ);

	తిరిగి NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
స్థిర nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    పూర్ణాంకం x, పూర్ణాంకం y,
		    నిర్మాణం mouse_action_state *mas)
{
	నిర్మాణం content *c = (నిర్మాణం content *)html;
	textarea_mouse_స్థితి ta_స్థితి;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ఎంపిక (mas->gadget.control->type) {
	స్థితి GADGET_SELECT:
		mas->ఫలితం.స్థితి_2 = messages_get("FormSelect");
		mas->ఫలితం.సూచిక = BROWSER_సూచిక_MENU;
		ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ఒకవేళ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ఖాళీ;
			}
			mas->ఫలితం.సూచిక = BROWSER_సూచిక_DEFAULT;
		} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
			సందేశం_దత్తాంశం.select_menu.gadget = mas->gadget.control;
			content_ప్రసారం(c,
					  CONTENT_సందేశం_SELECTMENU,
					  &సందేశం_దత్తాంశం);
		}
		ఆపు;

	స్థితి GADGET_CHECKBOX:
		mas->ఫలితం.స్థితి_2 = messages_get("FormCheckbox");
		ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_ఆదానం_element_సమితి_checked(
				(dom_html_ఆదానం_element *)(mas->gadget.control->కణుపు),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ఆపు;

	స్థితి GADGET_RADIO:
		mas->ఫలితం.స్థితి_2 = messages_get("FormRadio");
		ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_సమితి(mas->gadget.control);
		}
		ఆపు;

	స్థితి GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
			నిర్మాణం image_ఆదానం_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*coords));
			ఒకవేళ (coords == ఖాళీ) {
				తిరిగి NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ఒకవేళ (dom_కణుపు_సమితి_user_దత్తాంశం(
				mas->gadget.control->కణుపు,
				corestring_dom_ns_కీలకం_image_coords_కణుపు_దత్తాంశం,
				coords,
				html_image_coords_dom_user_దత్తాంశం_handler,
				&oldcoords) != DOM_NO_ERR) {
				తిరిగి NSERROR_OK;
			}
			విడుదల(oldcoords);
		}
		fallthrough;

	స్థితి GADGET_SUBMIT:
		ఒకవేళ (mas->gadget.control->form) {
			స్థిర అక్షరం స్థితి_బఫరు[200];

			snprintf(స్థితి_బఫరు,
				 పరిమాణం స్థితి_బఫరు,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->ఫలితం.స్థితి_2 = స్థితి_బఫరు;
			mas->ఫలితం.సూచిక = get_సూచిక_shape(mas->gadget.box,
								false);
			ఒకవేళ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->ఫలితం.action = ACTION_SUBMIT;
			}
		} లేకపోతే {
			mas->ఫలితం.స్థితి_2 = messages_get("FormBadSubmit");
		}
		ఆపు;

	స్థితి GADGET_TEXTBOX:
	స్థితి GADGET_PASSWORD:
	స్థితి GADGET_TEXTAREA:
		ఒకవేళ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->ఫలితం.స్థితి_2 = messages_get("FormTextarea");
		} లేకపోతే {
			mas->ఫలితం.స్థితి_2 = messages_get("FormTextbox");
		}

		ఒకవేళ (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			సంఘం html_selection_owner sel_owner;
			sel_owner.none = true;
			html_సమితి_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_స్థితి = textarea_mouse_action(mas->gadget.control->దత్తాంశం.పాఠ్యం.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		ఒకవేళ (ta_స్థితి & TEXTAREA_MOUSE_EDITOR) {
			mas->ఫలితం.సూచిక = get_సూచిక_shape(mas->gadget.box, false);
		} లేకపోతే {
			mas->ఫలితం.సూచిక = BROWSER_సూచిక_DEFAULT;
			mas->ఫలితం.స్థితి_2 = scrollbar_mouse_స్థితి_గమ్యం_సందేశం(ta_స్థితి >> 3);
		}
		ఆపు;

	స్థితి GADGET_HIDDEN:
		/* not possible: no box generated */
		ఆపు;

	స్థితి GADGET_RESET:
		mas->ఫలితం.స్థితి_2 = messages_get("FormReset");
		ఆపు;

	స్థితి GADGET_FILE:
		mas->ఫలితం.స్థితి_2 = messages_get("FormFile");
		ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
			సందేశం_దత్తాంశం.gadget_click.gadget = mas->gadget.control;
			content_ప్రసారం(c,
					  CONTENT_సందేశం_GADGETCLICK,
					  &సందేశం_దత్తాంశం);
		}
		ఆపు;

	స్థితి GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->ఫలితం.స్థితి_2 = messages_get("FormButton");
		ఆపు;
	}

	తిరిగి NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
స్థిర nserror
iframe_mouse_action(నిర్మాణం browser_కిటికీ *bw,
		    browser_mouse_state mouse,
		    పూర్ణాంకం x, పూర్ణాంకం y,
		    నిర్మాణం mouse_action_state *mas)
{
	పూర్ణాంకం pos_x, pos_y;
	దశాంశం scale;

	scale = browser_కిటికీ_get_scale(bw);

	browser_కిటికీ_get_position(mas->iframe, false, &pos_x, &pos_y);

	ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_కిటికీ_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} లేకపోతే {
		browser_కిటికీ_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->ఫలితం.action = ACTION_NOSEND;

	తిరిగి NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
స్థిర nserror
html_object_mouse_action(html_content *html,
			 నిర్మాణం browser_కిటికీ *bw,
			 browser_mouse_state mouse,
			 పూర్ణాంకం x, పూర్ణాంకం y,
			 నిర్మాణం mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ఒకవేళ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		సంఘం html_selection_owner sel_owner;
		sel_owner.none = true;
		html_సమితి_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} లేకపోతే {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->ఫలితం.action = ACTION_NOSEND;
	తిరిగి NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
స్థిర bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (scheme != ఖాళీ) {
		ఒకవేళ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	తిరిగి is_js;
}


/**
 * process mouse activity on a link
 */
స్థిర nserror
link_mouse_action(html_content *html,
		  నిర్మాణం browser_కిటికీ *bw,
		  browser_mouse_state mouse,
		  పూర్ణాంకం x, పూర్ణాంకం y,
		  నిర్మాణం mouse_action_state *mas)
{
	nserror res;
	అక్షరం *url_s = ఖాళీ;
	size_t url_l = 0;
	స్థిర అక్షరం స్థితి_బఫరు[200];
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

	ఒకవేళ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ఒకవేళ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ఖాళీ;
		}
	}

	ఒకవేళ (mas->title) {
		snprintf(స్థితి_బఫరు,
			 పరిమాణం స్థితి_బఫరు,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} లేకపోతే {
		snprintf(స్థితి_బఫరు,
			 పరిమాణం స్థితి_బఫరు,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ఒకవేళ (url_s != ఖాళీ) {
		విడుదల(url_s);
	}

	mas->ఫలితం.స్థితి_2 = స్థితి_బఫరు;

	mas->ఫలితం.సూచిక = get_సూచిక_shape(mas->link.box,
						mas->link.is_imagemap);

	ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_కిటికీ_navigate(bw,
					mas->link.url,
					content_get_url((నిర్మాణం content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ఖాళీ,
					ఖాళీ,
					ఖాళీ);

	} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		సందేశం_దత్తాంశం.savelink.url = mas->link.url;
		సందేశం_దత్తాంశం.savelink.title = mas->title;
		content_ప్రసారం((నిర్మాణం content *)html,
				  CONTENT_సందేశం_SAVELINK,
				  &సందేశం_దత్తాంశం);

	} లేకపోతే ఒకవేళ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ఒకవేళ (is_javascript_navigate_url(mas->link.url)) {
			mas->ఫలితం.action = ACTION_JS;
		} లేకపోతే {
			mas->ఫలితం.action = ACTION_NAVIGATE;
		}
	}

	తిరిగి NSERROR_OK;
}


స్థిర nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ఒకవేళ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		సంఘం html_focus_owner fo;
		fo.self = true;
		html_సమితి_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ఖాళీ);
	}

	తిరిగి NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
స్థిర nserror
default_mouse_action(html_content *html,
		  నిర్మాణం browser_కిటికీ *bw,
		  browser_mouse_state mouse,
		  పూర్ణాంకం x, పూర్ణాంకం y,
		  నిర్మాణం mouse_action_state *mas)
{
	నిర్మాణం content *c = (నిర్మాణం content *)html;

	/* frame resizing */
	ఒకవేళ (browser_కిటికీ_చట్రం_resize_ప్రారంభం(bw, mouse, x, y, &mas->ఫలితం.సూచిక)) {
		ఒకవేళ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->ఫలితం.స్థితి_2 = messages_get("FrameDrag");
		}
		తిరిగి default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	సంఘం html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ఒకవేళ (click && html->focus_type != HTML_FOCUS_SELF) {
		సంఘం html_focus_owner fo;
		fo.self = true;
		html_సమితి_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ఖాళీ);
	}
	ఒకవేళ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_సమితి_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ఒకవేళ (mas->పాఠ్యం.box) {
		పూర్ణాంకం pixel_స్థానభ్రంశం;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->పాఠ్యం.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->పాఠ్యం.box->పాఠ్యం,
				       mas->పాఠ్యం.box->నిడివి,
				       x - mas->పాఠ్యం.box_x,
				       &idx,
				       &pixel_స్థానభ్రంశం);

		ఒకవేళ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->పాఠ్యం.box->byte_స్థానభ్రంశం + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			సంఘం html_drag_owner drag_owner;

			ఒకవేళ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_సమితి_drag_type(html,
						   drag_type,
						   drag_owner,
						   ఖాళీ);
				mas->ఫలితం.స్థితి_2 = messages_get("Selecting");
			}

			ఒకవేళ (selection_చురుకైన(html->sel)) {
				sel_owner.none = false;
				html_సమితి_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} లేకపోతే ఒకవేళ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_సమితి_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			తిరిగి default_mouse_action_focus(html, mouse);
		}

	} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ఒకవేళ (selection_చురుకైన(html->sel)) {
		sel_owner.none = false;
		html_సమితి_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} లేకపోతే ఒకవేళ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_సమితి_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ఒకవేళ (mas->title) {
		mas->ఫలితం.స్థితి_2 = mas->title;
	}

	ఒకవేళ (mouse & BROWSER_MOUSE_DRAG_1) {
		ఒకవేళ (mouse & BROWSER_MOUSE_MOD_2) {
			సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
			సందేశం_దత్తాంశం.dragsave.type = CONTENT_SAVE_COMPLETE;
			సందేశం_దత్తాంశం.dragsave.content = ఖాళీ;
			content_ప్రసారం(c, CONTENT_సందేశం_DRAGSAVE, &సందేశం_దత్తాంశం);
		} లేకపోతే {
			ఒకవేళ (mas->drag_candidate == ఖాళీ) {
				browser_కిటికీ_పుట_drag_ప్రారంభం(bw, x, y);
			} లేకపోతే {
				html_box_drag_ప్రారంభం(mas->drag_candidate, x, y);
			}
			mas->ఫలితం.సూచిక = BROWSER_సూచిక_MOVE;
		}
	} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_DRAG_2) {
		ఒకవేళ (mouse & BROWSER_MOUSE_MOD_2) {
			సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
			సందేశం_దత్తాంశం.dragsave.type = CONTENT_SAVE_ఆధారం;
			సందేశం_దత్తాంశం.dragsave.content = ఖాళీ;
			content_ప్రసారం(c, CONTENT_సందేశం_DRAGSAVE, &సందేశం_దత్తాంశం);
		} లేకపోతే {
			ఒకవేళ (mas->drag_candidate == ఖాళీ) {
				browser_కిటికీ_పుట_drag_ప్రారంభం(bw, x, y);
			} లేకపోతే {
				html_box_drag_ప్రారంభం(mas->drag_candidate, x, y);
			}
			mas->ఫలితం.సూచిక = BROWSER_సూచిక_MOVE;
		}
	}


	తిరిగి default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
స్థిర nserror
mouse_action_drag_none(html_content *html,
		       నిర్మాణం browser_కిటికీ *bw,
		       browser_mouse_state mouse,
		       పూర్ణాంకం x, పూర్ణాంకం y)
{
	nserror res;
	నిర్మాణం content *c = (నిర్మాణం content *)html;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	lwc_string *మార్గం;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	స్థిర నిర్మాణం mouse_action_state mas;

	res = get_mouse_action_కణుపు(html, x, y, &mas);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.ఫలితం.action = ACTION_వెనుక;
	} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.ఫలితం.action = ACTION_FORWARD;
	} లేకపోతే ఒకవేళ (mas.scroll.bar) {
		mas.ఫలితం.స్థితి_2 = scrollbar_mouse_స్థితి_గమ్యం_సందేశం(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.ఫలితం.సూచిక = BROWSER_సూచిక_DEFAULT;

	} లేకపోతే ఒకవేళ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} లేకపోతే ఒకవేళ ((mas.object != ఖాళీ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ఒకవేళ (mouse & BROWSER_MOUSE_DRAG_2) {
			సందేశం_దత్తాంశం.dragsave.type = CONTENT_SAVE_NATIVE;
			సందేశం_దత్తాంశం.dragsave.content = mas.object;
			content_ప్రసారం(c, CONTENT_సందేశం_DRAGSAVE, &సందేశం_దత్తాంశం);

		} లేకపోతే ఒకవేళ (mouse & BROWSER_MOUSE_DRAG_1) {
			సందేశం_దత్తాంశం.dragsave.type = CONTENT_SAVE_ORIG;
			సందేశం_దత్తాంశం.dragsave.content = mas.object;
			content_ప్రసారం(c, CONTENT_సందేశం_DRAGSAVE, &సందేశం_దత్తాంశం);
		}

		/* \todo should have a drag-saving object msg */

	} లేకపోతే ఒకవేళ (mas.iframe != ఖాళీ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} లేకపోతే ఒకవేళ (mas.html_object.box != ఖాళీ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} లేకపోతే ఒకవేళ (mas.link.url != ఖాళీ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} లేకపోతే {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	/* send status and pointer message */
	ఒకవేళ (mas.ఫలితం.action != ACTION_NOSEND) {
		సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = mas.ఫలితం.స్థితి_2;
		content_ప్రసారం(c, CONTENT_సందేశం_స్థితి, &సందేశం_దత్తాంశం);

		సందేశం_దత్తాంశం.సూచిక = mas.ఫలితం.సూచిక;
		content_ప్రసారం(c, CONTENT_సందేశం_సూచిక, &సందేశం_దత్తాంశం);
	}

	/* fire dom click event */
	ఒకవేళ (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.కణుపు, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ఎంపిక (mas.ఫలితం.action) {
	స్థితి ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_కిటికీ_కనుగొను_లక్ష్యం(bw,
							     mas.gadget.లక్ష్యం,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ఆపు;

	స్థితి ACTION_NAVIGATE:
		res = browser_కిటికీ_navigate(
				browser_కిటికీ_కనుగొను_లక్ష్యం(bw,
							   mas.link.లక్ష్యం,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ఖాళీ,
				ఖాళీ,
				ఖాళీ);
		ఆపు;

	స్థితి ACTION_JS:
		మార్గం = nsurl_get_component(mas.link.url, NSURL_మార్గం);
		ఒకవేళ (మార్గం != ఖాళీ) {
			html_exec(c,
				  lwc_string_దత్తాంశం(మార్గం),
				  lwc_string_నిడివి(మార్గం));
			lwc_string_unref(మార్గం);
		}
		ఆపు;

	స్థితి ACTION_వెనుక:
		res = browser_కిటికీ_history_వెనుక(bw, false);
		ఆపు;

	స్థితి ACTION_FORWARD:
		res = browser_కిటికీ_history_forward(bw, false);
		ఆపు;

	స్థితి ACTION_NOSEND:
	స్థితి ACTION_NONE:
		res = NSERROR_OK;
		ఆపు;
	}

	తిరిగి res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(నిర్మాణం content *c,
			 నిర్మాణం browser_కిటికీ *bw,
			 browser_mouse_state mouse,
			 పూర్ణాంకం x, పూర్ణాంకం y)
{
	తిరిగి html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(నిర్మాణం content *c,
		  నిర్మాణం browser_కిటికీ *bw,
		  browser_mouse_state mouse,
		  పూర్ణాంకం x, పూర్ణాంకం y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ఒకవేళ (html->visible_select_menu != ఖాళీ) {
		తిరిగి mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ఎంపిక (html->drag_type) {
	స్థితి HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ఆపు;

	స్థితి HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ఆపు;

	స్థితి HTML_DRAG_TEXTAREA_SELECTION:
	స్థితి HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ఆపు;

	స్థితి HTML_DRAG_CONTENT_SELECTION:
	స్థితి HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ఆపు;

	స్థితి HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ఆపు;

	అప్రమేయ:
		/* Unknown content related drag type */
		ప్రతిజ్ఞా(0 && "Unknown content related drag type");
	}

	ఒకవేళ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	తిరిగి res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(నిర్మాణం content *c, uint32_t కీలకం)
{
	html_content *html = (html_content *) c;
	నిర్మాణం selection *sel = html->sel;

	/** \todo
	 * At the moment, the front end interface for keypress only gives
	 * us a UCS4 key value.  This doesn't doesn't have all the information
	 * we need to fill out the event properly.  We don't get to know about
	 * modifier keys, and things like CTRL+C are passed in as
	 * \ref NS_KEY_COPY_SELECTION, a magic value outside the valid Unicode
	 * range.
	 *
	 * We need to:
	 *
	 * 1. Update the front end interface so that both press and release
	 *    events reach the core.
	 * 2. Stop encoding the special keys like \ref NS_KEY_COPY_SELECTION as
	 *    magic values in the front ends, so we just get the events, e.g.:
	 *    1. Press ctrl
	 *    2. Press c
	 *    3. Release c
	 *    4. Release ctrl
	 * 3. Pass all the new info to the DOM KeyboardEvent events.
	 * 4. If there is a focused element, fire the event at that, instead of
	 *    `html->layout->node`.
	 * 5. Rebuild the \ref NS_KEY_COPY_SELECTION values from the info we
	 *    now get given, and use that for the code below this
	 *    \ref fire_dom_keyboard_event call.
	 * 6. Move the code after this \ref fire_dom_keyboard_event call into
	 *    the default action handler for DOM events.
	 *
	 * This will mean that if the JavaScript event listener does
	 * `event.preventDefault()` then we won't handle the event when
	 * we're not supposed to.
	 */
	ఒకవేళ (html->layout != ఖాళీ && html->layout->కణుపు != ఖాళీ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->కణుపు, true, true, కీలకం);
	}

	ఎంపిక (html->focus_type) {
	స్థితి HTML_FOCUS_CONTENT:
		తిరిగి content_keypress(html->focus_owner.content->object, కీలకం);

	స్థితి HTML_FOCUS_TEXTAREA:
		ఒకవేళ (box_textarea_keypress(html, html->focus_owner.textarea, కీలకం) == NSERROR_OK) {
			తిరిగి true;
		} లేకపోతే {
			తిరిగి false;
		}

	అప్రమేయ:
		/* Deal with it below */
		ఆపు;
	}

	ఎంపిక (కీలకం) {
	స్థితి NS_కీలకం_COPY_SELECTION:
		selection_copy_గమ్యం_clipboard(sel);
		తిరిగి true;

	స్థితి NS_కీలకం_CLEAR_SELECTION:
		selection_clear(sel, true);
		తిరిగి true;

	స్థితి NS_కీలకం_SELECT_ALL:
		selection_select_all(sel);
		తిరిగి true;

	స్థితి NS_కీలకం_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		తిరిగి selection_clear(sel, true);
	}

	తిరిగి false;
}


/**
 * Callback for in-page scrollbars.
 */
శూన్య html_అధికప్రవాహం_scroll_callback(శూన్య *client_దత్తాంశం,
		నిర్మాణం scrollbar_సందేశం_దత్తాంశం *scrollbar_దత్తాంశం)
{
	నిర్మాణం html_scrollbar_దత్తాంశం *దత్తాంశం = client_దత్తాంశం;
	html_content *html = (html_content *)దత్తాంశం->c;
	నిర్మాణం box *box = దత్తాంశం->box;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	html_drag_type drag_type;
	సంఘం html_drag_owner drag_owner;

	ఎంపిక(scrollbar_దత్తాంశం->సందేశం) {
	స్థితి SCROLLBAR_సందేశం_MOVED:

		ఒకవేళ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ఆపు;
		}

		html__redraw_a_box(html, box);
		ఆపు;
	స్థితి SCROLLBAR_సందేశం_SCROLL_ప్రారంభం:
	{
		నిర్మాణం rect rect = {
			.x0 = scrollbar_దత్తాంశం->x0,
			.y0 = scrollbar_దత్తాంశం->y0,
			.x1 = scrollbar_దత్తాంశం->x1,
			.y1 = scrollbar_దత్తాంశం->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_దత్తాంశం->scrollbar;
		html_సమితి_drag_type(html, drag_type, drag_owner, &rect);
	}
		ఆపు;
	స్థితి SCROLLBAR_సందేశం_SCROLL_ముగిసిన:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_సమితి_drag_type(html, drag_type, drag_owner, ఖాళీ);

		సందేశం_దత్తాంశం.సూచిక = BROWSER_సూచిక_AUTO;
		content_ప్రసారం(దత్తాంశం->c, CONTENT_సందేశం_సూచిక, &సందేశం_దత్తాంశం);
		ఆపు;
	}
}


/* Documented in html_internal.h */
శూన్య html_సమితి_drag_type(html_content *html, html_drag_type drag_type,
		సంఘం html_drag_owner drag_owner, స్థిరరాశి నిర్మాణం rect *rect)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

	ప్రతిజ్ఞా(html != ఖాళీ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ఎంపిక (drag_type) {
	స్థితి HTML_DRAG_NONE:
		ప్రతిజ్ఞా(drag_owner.no_owner == true);
		సందేశం_దత్తాంశం.drag.type = CONTENT_DRAG_NONE;
		ఆపు;

	స్థితి HTML_DRAG_SCROLLBAR:
	స్థితి HTML_DRAG_TEXTAREA_SCROLLBAR:
	స్థితి HTML_DRAG_CONTENT_SCROLL:
		సందేశం_దత్తాంశం.drag.type = CONTENT_DRAG_SCROLL;
		ఆపు;

	స్థితి HTML_DRAG_SELECTION:
		ప్రతిజ్ఞా(drag_owner.no_owner == true);
		fallthrough;
	స్థితి HTML_DRAG_TEXTAREA_SELECTION:
	స్థితి HTML_DRAG_CONTENT_SELECTION:
		సందేశం_దత్తాంశం.drag.type = CONTENT_DRAG_SELECTION;
		ఆపు;
	}
	సందేశం_దత్తాంశం.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_ప్రసారం((నిర్మాణం content *)html, CONTENT_సందేశం_DRAG, &సందేశం_దత్తాంశం);
}

/* Documented in html_internal.h */
శూన్య html_సమితి_focus(html_content *html, html_focus_type focus_type,
		సంఘం html_focus_owner focus_owner, bool hide_caret,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం ఎత్తు, స్థిరరాశి నిర్మాణం rect *clip)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	పూర్ణాంకం x_off = 0;
	పూర్ణాంకం y_off = 0;
	నిర్మాణం rect cr;
	bool textarea_కోల్పోయిన_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ప్రతిజ్ఞా(html != ఖాళీ);

	ఎంపిక (focus_type) {
	స్థితి HTML_FOCUS_SELF:
		ప్రతిజ్ఞా(focus_owner.self == true);
		ఒకవేళ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			తిరిగి;
		ఆపు;

	స్థితి HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ఆపు;

	స్థితి HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ఆపు;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ఒకవేళ (textarea_కోల్పోయిన_focus) {
		సందేశం_దత్తాంశం.caret.type = CONTENT_CARET_REMOVE;
	} లేకపోతే ఒకవేళ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		సందేశం_దత్తాంశం.caret.type = CONTENT_CARET_HIDE;
	} లేకపోతే {
		ఒకవేళ (clip != ఖాళీ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		సందేశం_దత్తాంశం.caret.type = CONTENT_CARET_సమితి_POS;
		సందేశం_దత్తాంశం.caret.pos.x = x + x_off;
		సందేశం_దత్తాంశం.caret.pos.y = y + y_off;
		సందేశం_దత్తాంశం.caret.pos.ఎత్తు = ఎత్తు;
		సందేశం_దత్తాంశం.caret.pos.clip = (clip == ఖాళీ) ? ఖాళీ : &cr;
	}

	/* Inform of the content's drag status change */
	content_ప్రసారం((నిర్మాణం content *)html, CONTENT_సందేశం_CARET, &సందేశం_దత్తాంశం);
}

/* Documented in html_internal.h */
శూన్య html_సమితి_selection(html_content *html, html_selection_type selection_type,
		సంఘం html_selection_owner selection_owner, bool read_only)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	నిర్మాణం box *box;
	bool మారింది = false;
	bool same_type = html->selection_type == selection_type;

	ప్రతిజ్ఞా(html != ఖాళీ);

	ఒకవేళ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		మారింది = true;

	/* Clear any existing selection */
	ఒకవేళ (html->selection_type != HTML_SELECTION_NONE) {
		ఎంపిక (html->selection_type) {
		స్థితి HTML_SELECTION_SELF:
			ఒకవేళ (same_type)
				ఆపు;
			selection_clear(html->sel, true);
			ఆపు;
		స్థితి HTML_SELECTION_TEXTAREA:
			ఒకవేళ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ఆపు;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->దత్తాంశం.పాఠ్యం.ta);
			ఆపు;
		స్థితి HTML_SELECTION_CONTENT:
			ఒకవేళ (same_type && html->selection_owner.content ==
					selection_owner.content)
				ఆపు;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ఆపు;
		అప్రమేయ:
			ఆపు;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ఒకవేళ (!మారింది)
		/* Don't need to report lack of change to owner */
		తిరిగి;

	/* Prepare msg */
	ఎంపిక (selection_type) {
	స్థితి HTML_SELECTION_NONE:
		ప్రతిజ్ఞా(selection_owner.none == true);
		సందేశం_దత్తాంశం.selection.selection = false;
		ఆపు;
	స్థితి HTML_SELECTION_SELF:
		ప్రతిజ్ఞా(selection_owner.none == false);
		fallthrough;
	స్థితి HTML_SELECTION_TEXTAREA:
	స్థితి HTML_SELECTION_CONTENT:
		సందేశం_దత్తాంశం.selection.selection = true;
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}
	సందేశం_దత్తాంశం.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_ప్రసారం((నిర్మాణం content *)html, CONTENT_సందేశం_SELECTION,
			&సందేశం_దత్తాంశం);
}
