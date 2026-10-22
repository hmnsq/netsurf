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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>

#شامل <dom/dom.h>

#شامل "utils/corestrings.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/mouse.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/layout.h"
#شامل "netsurf/keypress.h"
#شامل "content/hlcache.h"
#شامل "content/textsearch.h"
#شامل "desktop/browser_history.h"
#شامل "desktop/frames.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/selection.h"
#شامل "desktop/textarea.h"
#شامل "javascript/js.h"
#شامل "desktop/gui_internal.h"

#شامل "html/box.h"
#شامل "html/box_textarea.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"
#شامل "html/private.h"
#شامل "html/imagemap.h"
#شامل "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ساکن browser_اشارہ_گر_shape get_اشارہ_گر_shape(ساخت box *box, bool imagemap)
{
	browser_اشارہ_گر_shape اشارہ_گر;
	css_computed_style *style;
	شمار css_cursor_e cursor;
	lwc_string **cursor_uris;

	اگر (box->type == BOX_FLOAT_بایاں || box->type == BOX_FLOAT_دایاں)
		style = box->children->style;
	ورنہ
		style = box->style;

	اگر (style == تہی)
		واپس BROWSER_اشارہ_گر_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	انتخاب (cursor) {
	حالت CSS_CURSOR_AUTO:
		اگر (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			اشارہ_گر = BROWSER_اشارہ_گر_نقطہ;
		} ورنہ اگر (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			اشارہ_گر = BROWSER_اشارہ_گر_CARET;
		} ورنہ {
			/* html content doesn't mind */
			اشارہ_گر = BROWSER_اشارہ_گر_AUTO;
		}
		توڑو;
	حالت CSS_CURSOR_CROSSHAIR:
		اشارہ_گر = BROWSER_اشارہ_گر_CROSS;
		توڑو;
	حالت CSS_CURSOR_اشارہ_گر:
		اشارہ_گر = BROWSER_اشارہ_گر_نقطہ;
		توڑو;
	حالت CSS_CURSOR_MOVE:
		اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		توڑو;
	حالت CSS_CURSOR_E_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_دایاں;
		توڑو;
	حالت CSS_CURSOR_W_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_بایاں;
		توڑو;
	حالت CSS_CURSOR_N_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_اوپر;
		توڑو;
	حالت CSS_CURSOR_S_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_نیچے_کو;
		توڑو;
	حالت CSS_CURSOR_NE_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_RU;
		توڑو;
	حالت CSS_CURSOR_SW_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_LD;
		توڑو;
	حالت CSS_CURSOR_SE_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_RD;
		توڑو;
	حالت CSS_CURSOR_NW_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_LU;
		توڑو;
	حالت CSS_CURSOR_متن:
		اشارہ_گر = BROWSER_اشارہ_گر_CARET;
		توڑو;
	حالت CSS_CURSOR_WAIT:
		اشارہ_گر = BROWSER_اشارہ_گر_WAIT;
		توڑو;
	حالت CSS_CURSOR_پیش_رفت:
		اشارہ_گر = BROWSER_اشارہ_گر_پیش_رفت;
		توڑو;
	حالت CSS_CURSOR_HELP:
		اشارہ_گر = BROWSER_اشارہ_گر_HELP;
		توڑو;
	طےشدہ:
		اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
		توڑو;
	}

	واپس اشارہ_گر;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ساکن خالی html_box_drag_آغاز(ساخت box *box, صحیح x, صحیح y)
{
	صحیح box_x, box_y;
	صحیح scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	اگر (box->scroll_x != تہی) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				box->اونچائی + box->padding[BOTTOM] -
				SCROLLBAR_چوڑائی);
		scrollbar_آغاز_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} ورنہ اگر (box->scroll_y != تہی) {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				box->چوڑائی + box->padding[دایاں_2] -
				SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;

		scrollbar_آغاز_content_drag(box->scroll_y,
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
ساکن size_t html_selection_drag_انتہا(ساخت html_content *html,
		browser_mouse_state mouse, صحیح x, صحیح y, صحیح dir)
{
	صحیح pixel_انحراف;
	ساخت box *box;
	صحیح dx, dy;
	size_t idx = 0;

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	اگر (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->متن, box->لمبائی,
				       dx, &idx, &pixel_انحراف);

		idx += box->byte_انحراف;
	}

	واپس idx;
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
ساکن خالی
html_image_coords_dom_user_ڈیٹا_handler(dom_گرہ_operation operation,
					 dom_string *کلید,
					 خالی *ڈیٹا_2,
					 ساخت dom_گرہ *src,
					 ساخت dom_گرہ *dst)
{
	ساخت image_داخلہ_coords *oldcoords, *coords = ڈیٹا_2, *newcoords;

	اگر (!dom_string_isequal(corestring_dom_ns_کلید_image_coords_گرہ_ڈیٹا,
				کلید) || coords == تہی) {
		واپس;
	}

	انتخاب (operation) {
	حالت DOM_گرہ_CLONED:
		newcoords = صفر_حافظہ_دو(1, حجم(*newcoords));
		اگر (newcoords != تہی) {
			*newcoords = *coords;
			اگر (dom_گرہ_سیٹ_user_ڈیٹا(dst,
				 corestring_dom_ns_کلید_image_coords_گرہ_ڈیٹا,
				 newcoords,
				 html_image_coords_dom_user_ڈیٹا_handler,
				 &oldcoords) == DOM_NO_ERR) {
				آزاد(oldcoords);
			}
		}
		توڑو;

	حالت DOM_گرہ_DELETED:
		آزاد(coords);
		توڑو;

	حالت DOM_گرہ_RENAMED:
	حالت DOM_گرہ_IMPORTED:
	حالت DOM_گرہ_ADOPTED:
		توڑو;

	طےشدہ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
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
ساکن خالی
html_حد_سے_تجاوز_scroll_drag_انتہا(ساخت scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      صحیح x, صحیح y)
{
	صحیح scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ساخت html_scrollbar_ڈیٹا *ڈیٹا = scrollbar_get_ڈیٹا(scrollbar);
	ساخت box *box;

	box = ڈیٹا->box;
	box_coords(box, &box_x, &box_y);

	اگر (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				box->اونچائی + box->padding[BOTTOM] -
				SCROLLBAR_چوڑائی);
		scrollbar_mouse_drag_انتہا(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} ورنہ {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				box->چوڑائی + box->padding[دایاں_2] -
				SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_انتہا(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ساکن nserror
mouse_action_select_menu(html_content *html,
			 ساخت browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	ساخت box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;
	ثابت حرف *حالت_2;
	صحیح چوڑائی, اونچائی;
	ساخت hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	أكد(html->visible_select_menu != تہی);

	bw_drag_type = browser_کھڑکی_get_drag_type(bw);
	اگر (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_انتہا(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[بایاں_2].چوڑائی;
	box_y += box->اونچائی + box->border[BOTTOM].چوڑائی +
		box->padding[BOTTOM] + box->padding[چوٹی_2];

	حالت_2 = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	اگر (حالت_2 != تہی) {
		/* set status if menu still open */
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
		پیغام_ڈیٹا.explicit_حالت_متن = حالت_2;
		content_نشر((ساخت content *)html,
				  CONTENT_پیغام_حالت,
				  &پیغام_ڈیٹا);
		واپس NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &چوڑائی, &اونچائی);

	html->visible_select_menu = تہی;

	bw_content = browser_کھڑکی_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       چوڑائی,
			       اونچائی);
	واپس NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ساکن nserror
mouse_action_drag_selection(html_content *html,
			    ساخت browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساخت box *box;
	صحیح dir = -1;
	صحیح dx, dy;
	size_t idx;
	اتحاد html_drag_owner drag_owner;
	صحیح pixel_انحراف;
	plot_font_style_t fstyle;

	اگر (!mouse) {
		/* End of selection drag */
		اگر (selection_dragging_آغاز(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_انتہا(html, mouse, x, y, dir);

		اگر (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_سیٹ_drag_type(html, HTML_DRAG_NONE, drag_owner, تہی);

		واپس NSERROR_OK;
	}

	اگر (selection_dragging_آغاز(html->sel)) {
		dir = 1;
	}

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	اگر (box != تہی) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->متن,
				       box->لمبائی,
				       dx,
				       &idx,
				       &pixel_انحراف);

		selection_track(html->sel, mouse, box->byte_انحراف + idx);
	}
	واپس NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ساکن nserror
mouse_action_drag_scrollbar(html_content *html,
			    ساخت browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساخت scrollbar *scr;
	ساخت html_scrollbar_ڈیٹا *ڈیٹا;
	ساخت box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;
	ثابت حرف *حالت_2;
	صحیح scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_حالت scrollbar_حالت;

	scr = html->drag_owner.scrollbar;

	اگر (!mouse) {
		/* drag end: scrollbar */
		html_حد_سے_تجاوز_scroll_drag_انتہا(scr, mouse, x, y);
	}

	ڈیٹا = scrollbar_get_ڈیٹا(scr);

	box = ڈیٹا->box;

	box_coords(box, &box_x, &box_y);

	اگر (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				      box->اونچائی + box->padding[BOTTOM] -
				      SCROLLBAR_چوڑائی);
		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} ورنہ {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				      box->چوڑائی + box->padding[دایاں_2] -
				      SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;

		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	حالت_2 = scrollbar_mouse_حالت_مقصد_پیغام(scrollbar_حالت);

	اگر (حالت_2 != تہی) {
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

		پیغام_ڈیٹا.explicit_حالت_متن = حالت_2;
		content_نشر((ساخت content *)html,
				  CONTENT_پیغام_حالت,
				  &پیغام_ڈیٹا);
	}

	واپس NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ساکن nserror
mouse_action_drag_textarea(html_content *html,
			    ساخت browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساخت box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	box = html->drag_owner.textarea;

	أكد(box->gadget != تہی);
	أكد(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->ڈیٹا.متن.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	واپس NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ساکن nserror
mouse_action_drag_content(html_content *html,
			  ساخت browser_کھڑکی *bw,
			  browser_mouse_state mouse,
			  صحیح x, صحیح y)
{
	ساخت box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	box = html->drag_owner.content;
	أكد(box->object != تہی);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	واپس NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ساخت mouse_action_state {
	ساخت {
		ثابت حرف *حالت_2; /**< status text */
		browser_اشارہ_گر_shape اشارہ_گر; /**< pointer shape */
		شمار {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_عقب, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} نتیجہ;

	/** dom node */
	ساخت dom_گرہ *گرہ;

	/** html object */
	ساخت {
		ساخت box *box;
		صحیح pos_x;
		صحیح pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ساخت browser_کھڑکی *iframe;

	/** link either from href or imagemap */
	ساخت {
		ساخت box *box;
		nsurl *url;
		ثابت حرف *ہدف;
		bool is_imagemap;
	} link;

	/** gadget */
	ساخت {
		ساخت form_control *control;
		ساخت box *box;
		صحیح box_x;
		صحیح box_y;
		ثابت حرف *ہدف;
	} gadget;

	/** title */
	ثابت حرف *title;

	/** candidate box for drag operation */
	ساخت box *drag_candidate;

	/** scrollbar */
	ساخت {
		ساخت scrollbar *bar;
		صحیح mouse_x;
		صحیح mouse_y;
	} scroll;

	/** text in box */
	ساخت {
		ساخت box *box;
		صحیح box_x;
	} متن;
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
ساکن nserror
get_mouse_action_گرہ(html_content *html,
		      صحیح x, صحیح y,
		      ساخت mouse_action_state *man)
{
	ساخت box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, حجم(ساخت mouse_action_state));
	man->گرہ = html->layout->گرہ; /* Default dom node to the <HTML> */
	man->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[بایاں_2];
	box_y = box->margin[چوٹی_2];

	کرو {
		/* skip hidden boxes */
		اگر ((box->style != تہی) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			جاؤ اگلا_box;
		}

		اگر (box->گرہ != تہی) {
			man->گرہ = box->گرہ;
		}

		اگر (box->object) {
			اگر (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} ورنہ {
				man->object = box->object;
			}
		}

		اگر (box->iframe) {
			man->iframe = box->iframe;
		}

		اگر (box->href) {
			man->link.url = box->href;
			man->link.ہدف = box->ہدف;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		اگر (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.ہدف);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		اگر (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			اگر (box->gadget->form) {
				man->gadget.ہدف = box->gadget->form->ہدف;
			}
		}

		اگر (box->title) {
			man->title = box->title;
		}

		man->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(box, false);

		اگر ((box->scroll_x != تہی) ||
		    (box->scroll_y != تہی)) {
			صحیح padding_بایاں;
			صحیح padding_دایاں;
			صحیح padding_چوٹی;
			صحیح padding_bottom;

			اگر (man->drag_candidate == تہی) {
				man->drag_candidate = box;
			}

			padding_بایاں = box_x +
					scrollbar_get_انحراف(box->scroll_x);
			padding_دایاں = padding_بایاں + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2];
			padding_چوٹی = box_y +
					scrollbar_get_انحراف(box->scroll_y);
			padding_bottom = padding_چوٹی + box->padding[چوٹی_2] +
					box->اونچائی + box->padding[BOTTOM];

			اگر ((x > padding_بایاں) &&
			    (x < padding_دایاں) &&
			    (y > padding_چوٹی) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				اگر ((box->scroll_y != تہی) &&
				    (x > (padding_دایاں - SCROLLBAR_چوڑائی))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_دایاں - SCROLLBAR_چوڑائی);
					man->scroll.mouse_y = y - padding_چوٹی;
					توڑو;

				} ورنہ اگر ((box->scroll_x != تہی) &&
					   (y > (padding_bottom -
							SCROLLBAR_چوڑائی))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_بایاں;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_چوڑائی);
					توڑو;
				}
			}
		}

		اگر (box->متن && !box->object) {
			man->متن.box = box;
			man->متن.box_x = box_x;
		}

	اگلا_box:
		/* iterate to next box */
		box = box_at_نقطہ(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} جب (box != تہی);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	أكد(man->گرہ != تہی);

	واپس NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ساکن nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y,
		    ساخت mouse_action_state *mas)
{
	ساخت content *c = (ساخت content *)html;
	textarea_mouse_حالت ta_حالت;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	انتخاب (mas->gadget.control->type) {
	حالت GADGET_SELECT:
		mas->نتیجہ.حالت_2 = messages_get("FormSelect");
		mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MENU;
		اگر (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			اگر (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = تہی;
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
		} ورنہ اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			پیغام_ڈیٹا.select_menu.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_پیغام_SELECTMENU,
					  &پیغام_ڈیٹا);
		}
		توڑو;

	حالت GADGET_CHECKBOX:
		mas->نتیجہ.حالت_2 = messages_get("FormCheckbox");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_داخلہ_element_سیٹ_checked(
				(dom_html_داخلہ_element *)(mas->gadget.control->گرہ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		توڑو;

	حالت GADGET_RADIO:
		mas->نتیجہ.حالت_2 = messages_get("FormRadio");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_سیٹ(mas->gadget.control);
		}
		توڑو;

	حالت GADGET_IMAGE:
		/* This falls through to SUBMIT */
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			ساخت image_داخلہ_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = صفر_حافظہ_دو(1, حجم(*coords));
			اگر (coords == تہی) {
				واپس NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			اگر (dom_گرہ_سیٹ_user_ڈیٹا(
				mas->gadget.control->گرہ,
				corestring_dom_ns_کلید_image_coords_گرہ_ڈیٹا,
				coords,
				html_image_coords_dom_user_ڈیٹا_handler,
				&oldcoords) != DOM_NO_ERR) {
				واپس NSERROR_OK;
			}
			آزاد(oldcoords);
		}
		fallthrough;

	حالت GADGET_SUBMIT:
		اگر (mas->gadget.control->form) {
			ساکن حرف حالت_بفر[200];

			snprintf(حالت_بفر,
				 حجم حالت_بفر,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->نتیجہ.حالت_2 = حالت_بفر;
			mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->gadget.box,
								false);
			اگر (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->نتیجہ.action = ACTION_SUBMIT;
			}
		} ورنہ {
			mas->نتیجہ.حالت_2 = messages_get("FormBadSubmit");
		}
		توڑو;

	حالت GADGET_TEXTBOX:
	حالت GADGET_PASSWORD:
	حالت GADGET_TEXTAREA:
		اگر (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->نتیجہ.حالت_2 = messages_get("FormTextarea");
		} ورنہ {
			mas->نتیجہ.حالت_2 = messages_get("FormTextbox");
		}

		اگر (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;
			html_سیٹ_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_حالت = textarea_mouse_action(mas->gadget.control->ڈیٹا.متن.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		اگر (ta_حالت & TEXTAREA_MOUSE_EDITOR) {
			mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->gadget.box, false);
		} ورنہ {
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
			mas->نتیجہ.حالت_2 = scrollbar_mouse_حالت_مقصد_پیغام(ta_حالت >> 3);
		}
		توڑو;

	حالت GADGET_HIDDEN:
		/* not possible: no box generated */
		توڑو;

	حالت GADGET_RESET:
		mas->نتیجہ.حالت_2 = messages_get("FormReset");
		توڑو;

	حالت GADGET_FILE:
		mas->نتیجہ.حالت_2 = messages_get("FormFile");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			پیغام_ڈیٹا.gadget_click.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_پیغام_GADGETCLICK,
					  &پیغام_ڈیٹا);
		}
		توڑو;

	حالت GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->نتیجہ.حالت_2 = messages_get("FormButton");
		توڑو;
	}

	واپس NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ساکن nserror
iframe_mouse_action(ساخت browser_کھڑکی *bw,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y,
		    ساخت mouse_action_state *mas)
{
	صحیح pos_x, pos_y;
	اعشاری scale;

	scale = browser_کھڑکی_get_scale(bw);

	browser_کھڑکی_get_position(mas->iframe, false, &pos_x, &pos_y);

	اگر (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_کھڑکی_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} ورنہ {
		browser_کھڑکی_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->نتیجہ.action = ACTION_NOSEND;

	واپس NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ساکن nserror
html_object_mouse_action(html_content *html,
			 ساخت browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y,
			 ساخت mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	اگر (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		اتحاد html_selection_owner sel_owner;
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} ورنہ {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->نتیجہ.action = ACTION_NOSEND;
	واپس NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ساکن bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme != تہی) {
		اگر (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	واپس is_js;
}


/**
 * process mouse activity on a link
 */
ساکن nserror
link_mouse_action(html_content *html,
		  ساخت browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y,
		  ساخت mouse_action_state *mas)
{
	nserror res;
	حرف *url_s = تہی;
	size_t url_l = 0;
	ساکن حرف حالت_بفر[200];
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

	اگر (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		اگر (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = تہی;
		}
	}

	اگر (mas->title) {
		snprintf(حالت_بفر,
			 حجم حالت_بفر,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} ورنہ {
		snprintf(حالت_بفر,
			 حجم حالت_بفر,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	اگر (url_s != تہی) {
		آزاد(url_s);
	}

	mas->نتیجہ.حالت_2 = حالت_بفر;

	mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->link.box,
						mas->link.is_imagemap);

	اگر (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_کھڑکی_navigate(bw,
					mas->link.url,
					content_get_url((ساخت content *)html),
					BW_NAVIGATE_DOWNLOAD,
					تہی,
					تہی,
					تہی);

	} ورنہ اگر (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		پیغام_ڈیٹا.savelink.url = mas->link.url;
		پیغام_ڈیٹا.savelink.title = mas->title;
		content_نشر((ساخت content *)html,
				  CONTENT_پیغام_SAVELINK,
				  &پیغام_ڈیٹا);

	} ورنہ اگر (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		اگر (is_javascript_navigate_url(mas->link.url)) {
			mas->نتیجہ.action = ACTION_JS;
		} ورنہ {
			mas->نتیجہ.action = ACTION_NAVIGATE;
		}
	}

	واپس NSERROR_OK;
}


ساکن nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	اگر (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		اتحاد html_focus_owner fo;
		fo.self = true;
		html_سیٹ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, تہی);
	}

	واپس NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ساکن nserror
default_mouse_action(html_content *html,
		  ساخت browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y,
		  ساخت mouse_action_state *mas)
{
	ساخت content *c = (ساخت content *)html;

	/* frame resizing */
	اگر (browser_کھڑکی_چوکھٹا_resize_آغاز(bw, mouse, x, y, &mas->نتیجہ.اشارہ_گر)) {
		اگر (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->نتیجہ.حالت_2 = messages_get("FrameDrag");
		}
		واپس default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	اتحاد html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	اگر (click && html->focus_type != HTML_FOCUS_SELF) {
		اتحاد html_focus_owner fo;
		fo.self = true;
		html_سیٹ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, تہی);
	}
	اگر (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mas->متن.box) {
		صحیح pixel_انحراف;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->متن.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->متن.box->متن,
				       mas->متن.box->لمبائی,
				       x - mas->متن.box_x,
				       &idx,
				       &pixel_انحراف);

		اگر (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->متن.box->byte_انحراف + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			اتحاد html_drag_owner drag_owner;

			اگر (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_سیٹ_drag_type(html,
						   drag_type,
						   drag_owner,
						   تہی);
				mas->نتیجہ.حالت_2 = messages_get("Selecting");
			}

			اگر (selection_فعال(html->sel)) {
				sel_owner.none = false;
				html_سیٹ_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} ورنہ اگر (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_سیٹ_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			واپس default_mouse_action_focus(html, mouse);
		}

	} ورنہ اگر (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	اگر (selection_فعال(html->sel)) {
		sel_owner.none = false;
		html_سیٹ_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} ورنہ اگر (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mas->title) {
		mas->نتیجہ.حالت_2 = mas->title;
	}

	اگر (mouse & BROWSER_MOUSE_DRAG_1) {
		اگر (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
			پیغام_ڈیٹا.dragsave.type = CONTENT_SAVE_COMPLETE;
			پیغام_ڈیٹا.dragsave.content = تہی;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_ڈیٹا);
		} ورنہ {
			اگر (mas->drag_candidate == تہی) {
				browser_کھڑکی_صفحہ_drag_آغاز(bw, x, y);
			} ورنہ {
				html_box_drag_آغاز(mas->drag_candidate, x, y);
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	} ورنہ اگر (mouse & BROWSER_MOUSE_DRAG_2) {
		اگر (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
			پیغام_ڈیٹا.dragsave.type = CONTENT_SAVE_ماخذ;
			پیغام_ڈیٹا.dragsave.content = تہی;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_ڈیٹا);
		} ورنہ {
			اگر (mas->drag_candidate == تہی) {
				browser_کھڑکی_صفحہ_drag_آغاز(bw, x, y);
			} ورنہ {
				html_box_drag_آغاز(mas->drag_candidate, x, y);
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	}


	واپس default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ساکن nserror
mouse_action_drag_none(html_content *html,
		       ساخت browser_کھڑکی *bw,
		       browser_mouse_state mouse,
		       صحیح x, صحیح y)
{
	nserror res;
	ساخت content *c = (ساخت content *)html;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	lwc_string *راستہ;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ساکن ساخت mouse_action_state mas;

	res = get_mouse_action_گرہ(html, x, y, &mas);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	اگر (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.نتیجہ.action = ACTION_عقب;
	} ورنہ اگر (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.نتیجہ.action = ACTION_FORWARD;
	} ورنہ اگر (mas.scroll.bar) {
		mas.نتیجہ.حالت_2 = scrollbar_mouse_حالت_مقصد_پیغام(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;

	} ورنہ اگر (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} ورنہ اگر ((mas.object != تہی) && (mouse & BROWSER_MOUSE_MOD_2)) {

		اگر (mouse & BROWSER_MOUSE_DRAG_2) {
			پیغام_ڈیٹا.dragsave.type = CONTENT_SAVE_NATIVE;
			پیغام_ڈیٹا.dragsave.content = mas.object;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_ڈیٹا);

		} ورنہ اگر (mouse & BROWSER_MOUSE_DRAG_1) {
			پیغام_ڈیٹا.dragsave.type = CONTENT_SAVE_ORIG;
			پیغام_ڈیٹا.dragsave.content = mas.object;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_ڈیٹا);
		}

		/* \todo should have a drag-saving object msg */

	} ورنہ اگر (mas.iframe != تہی) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} ورنہ اگر (mas.html_object.box != تہی) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} ورنہ اگر (mas.link.url != تہی) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} ورنہ {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	/* send status and pointer message */
	اگر (mas.نتیجہ.action != ACTION_NOSEND) {
		پیغام_ڈیٹا.explicit_حالت_متن = mas.نتیجہ.حالت_2;
		content_نشر(c, CONTENT_پیغام_حالت, &پیغام_ڈیٹا);

		پیغام_ڈیٹا.اشارہ_گر = mas.نتیجہ.اشارہ_گر;
		content_نشر(c, CONTENT_پیغام_اشارہ_گر, &پیغام_ڈیٹا);
	}

	/* fire dom click event */
	اگر (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.گرہ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	انتخاب (mas.نتیجہ.action) {
	حالت ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_کھڑکی_ڈھونڈو_ہدف(bw,
							     mas.gadget.ہدف,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		توڑو;

	حالت ACTION_NAVIGATE:
		res = browser_کھڑکی_navigate(
				browser_کھڑکی_ڈھونڈو_ہدف(bw,
							   mas.link.ہدف,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				تہی,
				تہی,
				تہی);
		توڑو;

	حالت ACTION_JS:
		راستہ = nsurl_get_component(mas.link.url, NSURL_راستہ);
		اگر (راستہ != تہی) {
			html_exec(c,
				  lwc_string_ڈیٹا(راستہ),
				  lwc_string_لمبائی(راستہ));
			lwc_string_unref(راستہ);
		}
		توڑو;

	حالت ACTION_عقب:
		res = browser_کھڑکی_history_عقب(bw, false);
		توڑو;

	حالت ACTION_FORWARD:
		res = browser_کھڑکی_history_forward(bw, false);
		توڑو;

	حالت ACTION_NOSEND:
	حالت ACTION_NONE:
		res = NSERROR_OK;
		توڑو;
	}

	واپس res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ساخت content *c,
			 ساخت browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	واپس html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ساخت content *c,
		  ساخت browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	اگر (html->visible_select_menu != تہی) {
		واپس mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	انتخاب (html->drag_type) {
	حالت HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		توڑو;

	حالت HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		توڑو;

	حالت HTML_DRAG_TEXTAREA_SELECTION:
	حالت HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		توڑو;

	حالت HTML_DRAG_CONTENT_SELECTION:
	حالت HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		توڑو;

	حالت HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		توڑو;

	طےشدہ:
		/* Unknown content related drag type */
		أكد(0 && "Unknown content related drag type");
	}

	اگر (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	واپس res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ساخت content *c, uint32_t کلید)
{
	html_content *html = (html_content *) c;
	ساخت selection *sel = html->sel;

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
	اگر (html->layout != تہی && html->layout->گرہ != تہی) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->گرہ, true, true, کلید);
	}

	انتخاب (html->focus_type) {
	حالت HTML_FOCUS_CONTENT:
		واپس content_keypress(html->focus_owner.content->object, کلید);

	حالت HTML_FOCUS_TEXTAREA:
		اگر (box_textarea_keypress(html, html->focus_owner.textarea, کلید) == NSERROR_OK) {
			واپس true;
		} ورنہ {
			واپس false;
		}

	طےشدہ:
		/* Deal with it below */
		توڑو;
	}

	انتخاب (کلید) {
	حالت NS_کلید_COPY_SELECTION:
		selection_copy_مقصد_clipboard(sel);
		واپس true;

	حالت NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		واپس true;

	حالت NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		واپس true;

	حالت NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		واپس selection_clear(sel, true);
	}

	واپس false;
}


/**
 * Callback for in-page scrollbars.
 */
خالی html_حد_سے_تجاوز_scroll_callback(خالی *client_ڈیٹا,
		ساخت scrollbar_پیغام_ڈیٹا *scrollbar_ڈیٹا)
{
	ساخت html_scrollbar_ڈیٹا *ڈیٹا = client_ڈیٹا;
	html_content *html = (html_content *)ڈیٹا->c;
	ساخت box *box = ڈیٹا->box;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	html_drag_type drag_type;
	اتحاد html_drag_owner drag_owner;

	انتخاب(scrollbar_ڈیٹا->پیغام) {
	حالت SCROLLBAR_پیغام_MOVED:

		اگر (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			توڑو;
		}

		html__redraw_a_box(html, box);
		توڑو;
	حالت SCROLLBAR_پیغام_SCROLL_آغاز:
	{
		ساخت rect rect = {
			.x0 = scrollbar_ڈیٹا->x0,
			.y0 = scrollbar_ڈیٹا->y0,
			.x1 = scrollbar_ڈیٹا->x1,
			.y1 = scrollbar_ڈیٹا->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_ڈیٹا->scrollbar;
		html_سیٹ_drag_type(html, drag_type, drag_owner, &rect);
	}
		توڑو;
	حالت SCROLLBAR_پیغام_SCROLL_ختم_شدہ:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_سیٹ_drag_type(html, drag_type, drag_owner, تہی);

		پیغام_ڈیٹا.اشارہ_گر = BROWSER_اشارہ_گر_AUTO;
		content_نشر(ڈیٹا->c, CONTENT_پیغام_اشارہ_گر, &پیغام_ڈیٹا);
		توڑو;
	}
}


/* Documented in html_internal.h */
خالی html_سیٹ_drag_type(html_content *html, html_drag_type drag_type,
		اتحاد html_drag_owner drag_owner, ثابت ساخت rect *rect)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

	أكد(html != تہی);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	انتخاب (drag_type) {
	حالت HTML_DRAG_NONE:
		أكد(drag_owner.no_owner == true);
		پیغام_ڈیٹا.drag.type = CONTENT_DRAG_NONE;
		توڑو;

	حالت HTML_DRAG_SCROLLBAR:
	حالت HTML_DRAG_TEXTAREA_SCROLLBAR:
	حالت HTML_DRAG_CONTENT_SCROLL:
		پیغام_ڈیٹا.drag.type = CONTENT_DRAG_SCROLL;
		توڑو;

	حالت HTML_DRAG_SELECTION:
		أكد(drag_owner.no_owner == true);
		fallthrough;
	حالت HTML_DRAG_TEXTAREA_SELECTION:
	حالت HTML_DRAG_CONTENT_SELECTION:
		پیغام_ڈیٹا.drag.type = CONTENT_DRAG_SELECTION;
		توڑو;
	}
	پیغام_ڈیٹا.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_نشر((ساخت content *)html, CONTENT_پیغام_DRAG, &پیغام_ڈیٹا);
}

/* Documented in html_internal.h */
خالی html_سیٹ_focus(html_content *html, html_focus_type focus_type,
		اتحاد html_focus_owner focus_owner, bool hide_caret,
		صحیح x, صحیح y, صحیح اونچائی, ثابت ساخت rect *clip)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	صحیح x_off = 0;
	صحیح y_off = 0;
	ساخت rect cr;
	bool textarea_گمشدہ_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	أكد(html != تہی);

	انتخاب (focus_type) {
	حالت HTML_FOCUS_SELF:
		أكد(focus_owner.self == true);
		اگر (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			واپس;
		توڑو;

	حالت HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		توڑو;

	حالت HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		توڑو;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	اگر (textarea_گمشدہ_focus) {
		پیغام_ڈیٹا.caret.type = CONTENT_CARET_REMOVE;
	} ورنہ اگر (focus_type != HTML_FOCUS_SELF && hide_caret) {
		پیغام_ڈیٹا.caret.type = CONTENT_CARET_HIDE;
	} ورنہ {
		اگر (clip != تہی) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		پیغام_ڈیٹا.caret.type = CONTENT_CARET_سیٹ_POS;
		پیغام_ڈیٹا.caret.pos.x = x + x_off;
		پیغام_ڈیٹا.caret.pos.y = y + y_off;
		پیغام_ڈیٹا.caret.pos.اونچائی = اونچائی;
		پیغام_ڈیٹا.caret.pos.clip = (clip == تہی) ? تہی : &cr;
	}

	/* Inform of the content's drag status change */
	content_نشر((ساخت content *)html, CONTENT_پیغام_CARET, &پیغام_ڈیٹا);
}

/* Documented in html_internal.h */
خالی html_سیٹ_selection(html_content *html, html_selection_type selection_type,
		اتحاد html_selection_owner selection_owner, bool read_only)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	ساخت box *box;
	bool بدلا = false;
	bool same_type = html->selection_type == selection_type;

	أكد(html != تہی);

	اگر ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		بدلا = true;

	/* Clear any existing selection */
	اگر (html->selection_type != HTML_SELECTION_NONE) {
		انتخاب (html->selection_type) {
		حالت HTML_SELECTION_SELF:
			اگر (same_type)
				توڑو;
			selection_clear(html->sel, true);
			توڑو;
		حالت HTML_SELECTION_TEXTAREA:
			اگر (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				توڑو;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->ڈیٹا.متن.ta);
			توڑو;
		حالت HTML_SELECTION_CONTENT:
			اگر (same_type && html->selection_owner.content ==
					selection_owner.content)
				توڑو;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			توڑو;
		طےشدہ:
			توڑو;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	اگر (!بدلا)
		/* Don't need to report lack of change to owner */
		واپس;

	/* Prepare msg */
	انتخاب (selection_type) {
	حالت HTML_SELECTION_NONE:
		أكد(selection_owner.none == true);
		پیغام_ڈیٹا.selection.selection = false;
		توڑو;
	حالت HTML_SELECTION_SELF:
		أكد(selection_owner.none == false);
		fallthrough;
	حالت HTML_SELECTION_TEXTAREA:
	حالت HTML_SELECTION_CONTENT:
		پیغام_ڈیٹا.selection.selection = true;
		توڑو;
	طےشدہ:
		توڑو;
	}
	پیغام_ڈیٹا.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_نشر((ساخت content *)html, CONTENT_پیغام_SELECTION,
			&پیغام_ڈیٹا);
}
