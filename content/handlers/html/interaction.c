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

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <string.h>

#ضمن <dom/dom.h>

#ضمن "utils/corestrings.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/mouse.h"
#ضمن "netsurf/misc.h"
#ضمن "netsurf/layout.h"
#ضمن "netsurf/keypress.h"
#ضمن "content/hlcache.h"
#ضمن "content/textsearch.h"
#ضمن "desktop/browser_history.h"
#ضمن "desktop/frames.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/textarea.h"
#ضمن "javascript/js.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/box.h"
#ضمن "html/box_textarea.h"
#ضمن "html/box_inspect.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"
#ضمن "html/private.h"
#ضمن "html/imagemap.h"
#ضمن "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ساكن browser_اشارہ_گر_shape get_اشارہ_گر_shape(بنية box *box, bool imagemap)
{
	browser_اشارہ_گر_shape اشارہ_گر;
	css_computed_style *style;
	تعداد css_cursor_e cursor;
	lwc_string **cursor_uris;

	إذا (box->type == BOX_FLOAT_بایاں || box->type == BOX_FLOAT_دایاں)
		style = box->children->style;
	وإلا
		style = box->style;

	إذا (style == عدم)
		أرجع BROWSER_اشارہ_گر_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	بدل (cursor) {
	حالة CSS_CURSOR_AUTO:
		إذا (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			اشارہ_گر = BROWSER_اشارہ_گر_نقطہ;
		} وإلا إذا (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			اشارہ_گر = BROWSER_اشارہ_گر_CARET;
		} وإلا {
			/* html content doesn't mind */
			اشارہ_گر = BROWSER_اشارہ_گر_AUTO;
		}
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		اشارہ_گر = BROWSER_اشارہ_گر_CROSS;
		اكسر;
	حالة CSS_CURSOR_اشارہ_گر:
		اشارہ_گر = BROWSER_اشارہ_گر_نقطہ;
		اكسر;
	حالة CSS_CURSOR_MOVE:
		اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_دایاں;
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_بایاں;
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_اتے;
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_تھلے;
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_RU;
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_LD;
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_RD;
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		اشارہ_گر = BROWSER_اشارہ_گر_LU;
		اكسر;
	حالة CSS_CURSOR_متن:
		اشارہ_گر = BROWSER_اشارہ_گر_CARET;
		اكسر;
	حالة CSS_CURSOR_WAIT:
		اشارہ_گر = BROWSER_اشارہ_گر_WAIT;
		اكسر;
	حالة CSS_CURSOR_ترقی:
		اشارہ_گر = BROWSER_اشارہ_گر_ترقی;
		اكسر;
	حالة CSS_CURSOR_HELP:
		اشارہ_گر = BROWSER_اشارہ_گر_HELP;
		اكسر;
	افتراضي:
		اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
		اكسر;
	}

	أرجع اشارہ_گر;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ساكن فراغ html_box_drag_شروع(بنية box *box, صحيح x, صحيح y)
{
	صحيح box_x, box_y;
	صحيح scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	إذا (box->scroll_x != عدم) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				box->اچائی + box->padding[BOTTOM] -
				SCROLLBAR_چوڑائی);
		scrollbar_شروع_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} وإلا إذا (box->scroll_y != عدم) {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				box->چوڑائی + box->padding[دایاں_2] -
				SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;

		scrollbar_شروع_content_drag(box->scroll_y,
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
ساكن size_t html_selection_drag_سرا(بنية html_content *html,
		browser_mouse_state mouse, صحيح x, صحيح y, صحيح dir)
{
	صحيح pixel_وتھ;
	بنية box *box;
	صحيح dx, dy;
	size_t idx = 0;

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	إذا (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->متن, box->لمائی,
				       dx, &idx, &pixel_وتھ);

		idx += box->byte_وتھ;
	}

	أرجع idx;
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
ساكن فراغ
html_image_coords_dom_user_انکڑے_handler(dom_گرہ_operation operation,
					 dom_string *کلید,
					 فراغ *انکڑے_2,
					 بنية dom_گرہ *src,
					 بنية dom_گرہ *dst)
{
	بنية image_داخلہ_coords *oldcoords, *coords = انکڑے_2, *newcoords;

	إذا (!dom_string_isequal(corestring_dom_ns_کلید_image_coords_گرہ_انکڑے,
				کلید) || coords == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_گرہ_CLONED:
		newcoords = صفر_حافظہ_دیو(1, حجم(*newcoords));
		إذا (newcoords != عدم) {
			*newcoords = *coords;
			إذا (dom_گرہ_سیٹ_user_انکڑے(dst,
				 corestring_dom_ns_کلید_image_coords_گرہ_انکڑے,
				 newcoords,
				 html_image_coords_dom_user_انکڑے_handler,
				 &oldcoords) == DOM_NO_ERR) {
				حرر(oldcoords);
			}
		}
		اكسر;

	حالة DOM_گرہ_DELETED:
		حرر(coords);
		اكسر;

	حالة DOM_گرہ_RENAMED:
	حالة DOM_گرہ_IMPORTED:
	حالة DOM_گرہ_ADOPTED:
		اكسر;

	افتراضي:
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
ساكن فراغ
html_حد_توں_ودھ_scroll_drag_سرا(بنية scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      صحيح x, صحيح y)
{
	صحيح scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	بنية html_scrollbar_انکڑے *انکڑے = scrollbar_get_انکڑے(scrollbar);
	بنية box *box;

	box = انکڑے->box;
	box_coords(box, &box_x, &box_y);

	إذا (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				box->اچائی + box->padding[BOTTOM] -
				SCROLLBAR_چوڑائی);
		scrollbar_mouse_drag_سرا(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} وإلا {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				box->چوڑائی + box->padding[دایاں_2] -
				SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_سرا(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ساكن nserror
mouse_action_select_menu(html_content *html,
			 بنية browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;
	ثابت حرف *حالت;
	صحيح چوڑائی, اچائی;
	بنية hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	أكد(html->visible_select_menu != عدم);

	bw_drag_type = browser_کھڑکی_get_drag_type(bw);
	إذا (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_سرا(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[بایاں_2].چوڑائی;
	box_y += box->اچائی + box->border[BOTTOM].چوڑائی +
		box->padding[BOTTOM] + box->padding[چوٹی_2];

	حالت = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	إذا (حالت != عدم) {
		/* set status if menu still open */
		اتحاد content_پیغام_انکڑے پیغام_انکڑے;
		پیغام_انکڑے.explicit_حالت_متن = حالت;
		content_نشر((بنية content *)html,
				  CONTENT_پیغام_حالت,
				  &پیغام_انکڑے);
		أرجع NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &چوڑائی, &اچائی);

	html->visible_select_menu = عدم;

	bw_content = browser_کھڑکی_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       چوڑائی,
			       اچائی);
	أرجع NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ساكن nserror
mouse_action_drag_selection(html_content *html,
			    بنية browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحيح x, صحيح y)
{
	بنية box *box;
	صحيح dir = -1;
	صحيح dx, dy;
	size_t idx;
	اتحاد html_drag_owner drag_owner;
	صحيح pixel_وتھ;
	plot_font_style_t fstyle;

	إذا (!mouse) {
		/* End of selection drag */
		إذا (selection_dragging_شروع(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_سرا(html, mouse, x, y, dir);

		إذا (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_سیٹ_drag_type(html, HTML_DRAG_NONE, drag_owner, عدم);

		أرجع NSERROR_OK;
	}

	إذا (selection_dragging_شروع(html->sel)) {
		dir = 1;
	}

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	إذا (box != عدم) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->متن,
				       box->لمائی,
				       dx,
				       &idx,
				       &pixel_وتھ);

		selection_track(html->sel, mouse, box->byte_وتھ + idx);
	}
	أرجع NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ساكن nserror
mouse_action_drag_scrollbar(html_content *html,
			    بنية browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحيح x, صحيح y)
{
	بنية scrollbar *scr;
	بنية html_scrollbar_انکڑے *انکڑے;
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;
	ثابت حرف *حالت;
	صحيح scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_حالت scrollbar_حالت;

	scr = html->drag_owner.scrollbar;

	إذا (!mouse) {
		/* drag end: scrollbar */
		html_حد_توں_ودھ_scroll_drag_سرا(scr, mouse, x, y);
	}

	انکڑے = scrollbar_get_انکڑے(scr);

	box = انکڑے->box;

	box_coords(box, &box_x, &box_y);

	إذا (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٹی_2] +
				      box->اچائی + box->padding[BOTTOM] -
				      SCROLLBAR_چوڑائی);
		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} وإلا {
		scroll_mouse_x = x - (box_x + box->padding[بایاں_2] +
				      box->چوڑائی + box->padding[دایاں_2] -
				      SCROLLBAR_چوڑائی);
		scroll_mouse_y = y - box_y;

		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	حالت = scrollbar_mouse_حالت_منزل_سنیہا(scrollbar_حالت);

	إذا (حالت != عدم) {
		اتحاد content_پیغام_انکڑے پیغام_انکڑے;

		پیغام_انکڑے.explicit_حالت_متن = حالت;
		content_نشر((بنية content *)html,
				  CONTENT_پیغام_حالت,
				  &پیغام_انکڑے);
	}

	أرجع NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ساكن nserror
mouse_action_drag_textarea(html_content *html,
			    بنية browser_کھڑکی *bw,
			    browser_mouse_state mouse,
			    صحيح x, صحيح y)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;

	box = html->drag_owner.textarea;

	أكد(box->gadget != عدم);
	أكد(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->انکڑے.متن.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	أرجع NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ساكن nserror
mouse_action_drag_content(html_content *html,
			  بنية browser_کھڑکی *bw,
			  browser_mouse_state mouse,
			  صحيح x, صحيح y)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;

	box = html->drag_owner.content;
	أكد(box->object != عدم);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	أرجع NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
بنية mouse_action_state {
	بنية {
		ثابت حرف *حالت; /**< status text */
		browser_اشارہ_گر_shape اشارہ_گر; /**< pointer shape */
		تعداد {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_پٹھ, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} نتیجہ;

	/** dom node */
	بنية dom_گرہ *گرہ;

	/** html object */
	بنية {
		بنية box *box;
		صحيح pos_x;
		صحيح pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	بنية browser_کھڑکی *iframe;

	/** link either from href or imagemap */
	بنية {
		بنية box *box;
		nsurl *url;
		ثابت حرف *نشانہ;
		bool is_imagemap;
	} link;

	/** gadget */
	بنية {
		بنية form_control *control;
		بنية box *box;
		صحيح box_x;
		صحيح box_y;
		ثابت حرف *نشانہ;
	} gadget;

	/** title */
	ثابت حرف *title;

	/** candidate box for drag operation */
	بنية box *drag_candidate;

	/** scrollbar */
	بنية {
		بنية scrollbar *bar;
		صحيح mouse_x;
		صحيح mouse_y;
	} scroll;

	/** text in box */
	بنية {
		بنية box *box;
		صحيح box_x;
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
ساكن nserror
get_mouse_action_گرہ(html_content *html,
		      صحيح x, صحيح y,
		      بنية mouse_action_state *man)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, حجم(بنية mouse_action_state));
	man->گرہ = html->layout->گرہ; /* Default dom node to the <HTML> */
	man->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[بایاں_2];
	box_y = box->margin[چوٹی_2];

	افعل {
		/* skip hidden boxes */
		إذا ((box->style != عدم) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			اذهب اگلا_box;
		}

		إذا (box->گرہ != عدم) {
			man->گرہ = box->گرہ;
		}

		إذا (box->object) {
			إذا (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} وإلا {
				man->object = box->object;
			}
		}

		إذا (box->iframe) {
			man->iframe = box->iframe;
		}

		إذا (box->href) {
			man->link.url = box->href;
			man->link.نشانہ = box->نشانہ;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		إذا (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.نشانہ);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		إذا (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			إذا (box->gadget->form) {
				man->gadget.نشانہ = box->gadget->form->نشانہ;
			}
		}

		إذا (box->title) {
			man->title = box->title;
		}

		man->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(box, false);

		إذا ((box->scroll_x != عدم) ||
		    (box->scroll_y != عدم)) {
			صحيح padding_بایاں;
			صحيح padding_دایاں;
			صحيح padding_چوٹی;
			صحيح padding_bottom;

			إذا (man->drag_candidate == عدم) {
				man->drag_candidate = box;
			}

			padding_بایاں = box_x +
					scrollbar_get_وتھ(box->scroll_x);
			padding_دایاں = padding_بایاں + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2];
			padding_چوٹی = box_y +
					scrollbar_get_وتھ(box->scroll_y);
			padding_bottom = padding_چوٹی + box->padding[چوٹی_2] +
					box->اچائی + box->padding[BOTTOM];

			إذا ((x > padding_بایاں) &&
			    (x < padding_دایاں) &&
			    (y > padding_چوٹی) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				إذا ((box->scroll_y != عدم) &&
				    (x > (padding_دایاں - SCROLLBAR_چوڑائی))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_دایاں - SCROLLBAR_چوڑائی);
					man->scroll.mouse_y = y - padding_چوٹی;
					اكسر;

				} وإلا إذا ((box->scroll_x != عدم) &&
					   (y > (padding_bottom -
							SCROLLBAR_چوڑائی))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_بایاں;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_چوڑائی);
					اكسر;
				}
			}
		}

		إذا (box->متن && !box->object) {
			man->متن.box = box;
			man->متن.box_x = box_x;
		}

	اگلا_box:
		/* iterate to next box */
		box = box_at_نقطہ(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} طالما (box != عدم);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	أكد(man->گرہ != عدم);

	أرجع NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ساكن nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    صحيح x, صحيح y,
		    بنية mouse_action_state *mas)
{
	بنية content *c = (بنية content *)html;
	textarea_mouse_حالت ta_حالت;
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	بدل (mas->gadget.control->type) {
	حالة GADGET_SELECT:
		mas->نتیجہ.حالت = messages_get("FormSelect");
		mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MENU;
		إذا (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			إذا (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = عدم;
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
		} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			پیغام_انکڑے.select_menu.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_پیغام_SELECTMENU,
					  &پیغام_انکڑے);
		}
		اكسر;

	حالة GADGET_CHECKBOX:
		mas->نتیجہ.حالت = messages_get("FormCheckbox");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_داخلہ_element_سیٹ_checked(
				(dom_html_داخلہ_element *)(mas->gadget.control->گرہ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		اكسر;

	حالة GADGET_RADIO:
		mas->نتیجہ.حالت = messages_get("FormRadio");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_سیٹ(mas->gadget.control);
		}
		اكسر;

	حالة GADGET_IMAGE:
		/* This falls through to SUBMIT */
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			بنية image_داخلہ_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = صفر_حافظہ_دیو(1, حجم(*coords));
			إذا (coords == عدم) {
				أرجع NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			إذا (dom_گرہ_سیٹ_user_انکڑے(
				mas->gadget.control->گرہ,
				corestring_dom_ns_کلید_image_coords_گرہ_انکڑے,
				coords,
				html_image_coords_dom_user_انکڑے_handler,
				&oldcoords) != DOM_NO_ERR) {
				أرجع NSERROR_OK;
			}
			حرر(oldcoords);
		}
		fallthrough;

	حالة GADGET_SUBMIT:
		إذا (mas->gadget.control->form) {
			ساكن حرف حالت_بفر[200];

			snprintf(حالت_بفر,
				 حجم حالت_بفر,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->نتیجہ.حالت = حالت_بفر;
			mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->gadget.box,
								false);
			إذا (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->نتیجہ.action = ACTION_SUBMIT;
			}
		} وإلا {
			mas->نتیجہ.حالت = messages_get("FormBadSubmit");
		}
		اكسر;

	حالة GADGET_TEXTBOX:
	حالة GADGET_PASSWORD:
	حالة GADGET_TEXTAREA:
		إذا (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->نتیجہ.حالت = messages_get("FormTextarea");
		} وإلا {
			mas->نتیجہ.حالت = messages_get("FormTextbox");
		}

		إذا (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;
			html_سیٹ_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_حالت = textarea_mouse_action(mas->gadget.control->انکڑے.متن.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		إذا (ta_حالت & TEXTAREA_MOUSE_EDITOR) {
			mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->gadget.box, false);
		} وإلا {
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;
			mas->نتیجہ.حالت = scrollbar_mouse_حالت_منزل_سنیہا(ta_حالت >> 3);
		}
		اكسر;

	حالة GADGET_HIDDEN:
		/* not possible: no box generated */
		اكسر;

	حالة GADGET_RESET:
		mas->نتیجہ.حالت = messages_get("FormReset");
		اكسر;

	حالة GADGET_FILE:
		mas->نتیجہ.حالت = messages_get("FormFile");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			پیغام_انکڑے.gadget_click.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_پیغام_GADGETCLICK,
					  &پیغام_انکڑے);
		}
		اكسر;

	حالة GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->نتیجہ.حالت = messages_get("FormButton");
		اكسر;
	}

	أرجع NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ساكن nserror
iframe_mouse_action(بنية browser_کھڑکی *bw,
		    browser_mouse_state mouse,
		    صحيح x, صحيح y,
		    بنية mouse_action_state *mas)
{
	صحيح pos_x, pos_y;
	عائم scale;

	scale = browser_کھڑکی_get_scale(bw);

	browser_کھڑکی_get_position(mas->iframe, false, &pos_x, &pos_y);

	إذا (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_کھڑکی_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} وإلا {
		browser_کھڑکی_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->نتیجہ.action = ACTION_NOSEND;

	أرجع NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ساكن nserror
html_object_mouse_action(html_content *html,
			 بنية browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y,
			 بنية mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	إذا (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		اتحاد html_selection_owner sel_owner;
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	إذا (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} وإلا {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->نتیجہ.action = ACTION_NOSEND;
	أرجع NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ساكن bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	إذا (scheme != عدم) {
		إذا (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	أرجع is_js;
}


/**
 * process mouse activity on a link
 */
ساكن nserror
link_mouse_action(html_content *html,
		  بنية browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحيح x, صحيح y,
		  بنية mouse_action_state *mas)
{
	nserror res;
	حرف *url_s = عدم;
	size_t url_l = 0;
	ساكن حرف حالت_بفر[200];
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;

	إذا (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		إذا (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = عدم;
		}
	}

	إذا (mas->title) {
		snprintf(حالت_بفر,
			 حجم حالت_بفر,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} وإلا {
		snprintf(حالت_بفر,
			 حجم حالت_بفر,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	إذا (url_s != عدم) {
		حرر(url_s);
	}

	mas->نتیجہ.حالت = حالت_بفر;

	mas->نتیجہ.اشارہ_گر = get_اشارہ_گر_shape(mas->link.box,
						mas->link.is_imagemap);

	إذا (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_کھڑکی_navigate(bw,
					mas->link.url,
					content_get_url((بنية content *)html),
					BW_NAVIGATE_DOWNLOAD,
					عدم,
					عدم,
					عدم);

	} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		پیغام_انکڑے.savelink.url = mas->link.url;
		پیغام_انکڑے.savelink.title = mas->title;
		content_نشر((بنية content *)html,
				  CONTENT_پیغام_SAVELINK,
				  &پیغام_انکڑے);

	} وإلا إذا (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		إذا (is_javascript_navigate_url(mas->link.url)) {
			mas->نتیجہ.action = ACTION_JS;
		} وإلا {
			mas->نتیجہ.action = ACTION_NAVIGATE;
		}
	}

	أرجع NSERROR_OK;
}


ساكن nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	إذا (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		اتحاد html_focus_owner fo;
		fo.self = true;
		html_سیٹ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, عدم);
	}

	أرجع NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ساكن nserror
default_mouse_action(html_content *html,
		  بنية browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحيح x, صحيح y,
		  بنية mouse_action_state *mas)
{
	بنية content *c = (بنية content *)html;

	/* frame resizing */
	إذا (browser_کھڑکی_چوکھٹا_resize_شروع(bw, mouse, x, y, &mas->نتیجہ.اشارہ_گر)) {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->نتیجہ.حالت = messages_get("FrameDrag");
		}
		أرجع default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	اتحاد html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	إذا (click && html->focus_type != HTML_FOCUS_SELF) {
		اتحاد html_focus_owner fo;
		fo.self = true;
		html_سیٹ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, عدم);
	}
	إذا (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	إذا (mas->متن.box) {
		صحيح pixel_وتھ;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->متن.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->متن.box->متن,
				       mas->متن.box->لمائی,
				       x - mas->متن.box_x,
				       &idx,
				       &pixel_وتھ);

		إذا (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->متن.box->byte_وتھ + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			اتحاد html_drag_owner drag_owner;

			إذا (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_سیٹ_drag_type(html,
						   drag_type,
						   drag_owner,
						   عدم);
				mas->نتیجہ.حالت = messages_get("Selecting");
			}

			إذا (selection_سرگرم(html->sel)) {
				sel_owner.none = false;
				html_سیٹ_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} وإلا إذا (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_سیٹ_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			أرجع default_mouse_action_focus(html, mouse);
		}

	} وإلا إذا (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	إذا (selection_سرگرم(html->sel)) {
		sel_owner.none = false;
		html_سیٹ_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} وإلا إذا (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_سیٹ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	إذا (mas->title) {
		mas->نتیجہ.حالت = mas->title;
	}

	إذا (mouse & BROWSER_MOUSE_DRAG_1) {
		إذا (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_پیغام_انکڑے پیغام_انکڑے;
			پیغام_انکڑے.dragsave.type = CONTENT_SAVE_COMPLETE;
			پیغام_انکڑے.dragsave.content = عدم;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_انکڑے);
		} وإلا {
			إذا (mas->drag_candidate == عدم) {
				browser_کھڑکی_ورقہ_drag_شروع(bw, x, y);
			} وإلا {
				html_box_drag_شروع(mas->drag_candidate, x, y);
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	} وإلا إذا (mouse & BROWSER_MOUSE_DRAG_2) {
		إذا (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_پیغام_انکڑے پیغام_انکڑے;
			پیغام_انکڑے.dragsave.type = CONTENT_SAVE_سروت;
			پیغام_انکڑے.dragsave.content = عدم;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_انکڑے);
		} وإلا {
			إذا (mas->drag_candidate == عدم) {
				browser_کھڑکی_ورقہ_drag_شروع(bw, x, y);
			} وإلا {
				html_box_drag_شروع(mas->drag_candidate, x, y);
			}
			mas->نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_MOVE;
		}
	}


	أرجع default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ساكن nserror
mouse_action_drag_none(html_content *html,
		       بنية browser_کھڑکی *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	nserror res;
	بنية content *c = (بنية content *)html;
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	lwc_string *رستہ;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ساكن بنية mouse_action_state mas;

	res = get_mouse_action_گرہ(html, x, y, &mas);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.نتیجہ.action = ACTION_پٹھ;
	} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.نتیجہ.action = ACTION_FORWARD;
	} وإلا إذا (mas.scroll.bar) {
		mas.نتیجہ.حالت = scrollbar_mouse_حالت_منزل_سنیہا(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.نتیجہ.اشارہ_گر = BROWSER_اشارہ_گر_DEFAULT;

	} وإلا إذا (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} وإلا إذا ((mas.object != عدم) && (mouse & BROWSER_MOUSE_MOD_2)) {

		إذا (mouse & BROWSER_MOUSE_DRAG_2) {
			پیغام_انکڑے.dragsave.type = CONTENT_SAVE_NATIVE;
			پیغام_انکڑے.dragsave.content = mas.object;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_انکڑے);

		} وإلا إذا (mouse & BROWSER_MOUSE_DRAG_1) {
			پیغام_انکڑے.dragsave.type = CONTENT_SAVE_ORIG;
			پیغام_انکڑے.dragsave.content = mas.object;
			content_نشر(c, CONTENT_پیغام_DRAGSAVE, &پیغام_انکڑے);
		}

		/* \todo should have a drag-saving object msg */

	} وإلا إذا (mas.iframe != عدم) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} وإلا إذا (mas.html_object.box != عدم) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} وإلا إذا (mas.link.url != عدم) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} وإلا {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* send status and pointer message */
	إذا (mas.نتیجہ.action != ACTION_NOSEND) {
		پیغام_انکڑے.explicit_حالت_متن = mas.نتیجہ.حالت;
		content_نشر(c, CONTENT_پیغام_حالت, &پیغام_انکڑے);

		پیغام_انکڑے.اشارہ_گر = mas.نتیجہ.اشارہ_گر;
		content_نشر(c, CONTENT_پیغام_اشارہ_گر, &پیغام_انکڑے);
	}

	/* fire dom click event */
	إذا (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.گرہ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	بدل (mas.نتیجہ.action) {
	حالة ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_کھڑکی_لبھو_نشانہ(bw,
							     mas.gadget.نشانہ,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		اكسر;

	حالة ACTION_NAVIGATE:
		res = browser_کھڑکی_navigate(
				browser_کھڑکی_لبھو_نشانہ(bw,
							   mas.link.نشانہ,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				عدم,
				عدم,
				عدم);
		اكسر;

	حالة ACTION_JS:
		رستہ = nsurl_get_component(mas.link.url, NSURL_رستہ);
		إذا (رستہ != عدم) {
			html_exec(c,
				  lwc_string_انکڑے(رستہ),
				  lwc_string_لمائی(رستہ));
			lwc_string_unref(رستہ);
		}
		اكسر;

	حالة ACTION_پٹھ:
		res = browser_کھڑکی_history_پٹھ(bw, false);
		اكسر;

	حالة ACTION_FORWARD:
		res = browser_کھڑکی_history_forward(bw, false);
		اكسر;

	حالة ACTION_NOSEND:
	حالة ACTION_NONE:
		res = NSERROR_OK;
		اكسر;
	}

	أرجع res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(بنية content *c,
			 بنية browser_کھڑکی *bw,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	أرجع html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(بنية content *c,
		  بنية browser_کھڑکی *bw,
		  browser_mouse_state mouse,
		  صحيح x, صحيح y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	إذا (html->visible_select_menu != عدم) {
		أرجع mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	بدل (html->drag_type) {
	حالة HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		اكسر;

	حالة HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		اكسر;

	حالة HTML_DRAG_TEXTAREA_SELECTION:
	حالة HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		اكسر;

	حالة HTML_DRAG_CONTENT_SELECTION:
	حالة HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		اكسر;

	حالة HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		اكسر;

	افتراضي:
		/* Unknown content related drag type */
		أكد(0 && "Unknown content related drag type");
	}

	إذا (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	أرجع res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(بنية content *c, uint32_t کلید)
{
	html_content *html = (html_content *) c;
	بنية selection *sel = html->sel;

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
	إذا (html->layout != عدم && html->layout->گرہ != عدم) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->گرہ, true, true, کلید);
	}

	بدل (html->focus_type) {
	حالة HTML_FOCUS_CONTENT:
		أرجع content_keypress(html->focus_owner.content->object, کلید);

	حالة HTML_FOCUS_TEXTAREA:
		إذا (box_textarea_keypress(html, html->focus_owner.textarea, کلید) == NSERROR_OK) {
			أرجع true;
		} وإلا {
			أرجع false;
		}

	افتراضي:
		/* Deal with it below */
		اكسر;
	}

	بدل (کلید) {
	حالة NS_کلید_COPY_SELECTION:
		selection_copy_منزل_clipboard(sel);
		أرجع true;

	حالة NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		أرجع selection_clear(sel, true);
	}

	أرجع false;
}


/**
 * Callback for in-page scrollbars.
 */
فراغ html_حد_توں_ودھ_scroll_callback(فراغ *client_انکڑے,
		بنية scrollbar_پیغام_انکڑے *scrollbar_انکڑے)
{
	بنية html_scrollbar_انکڑے *انکڑے = client_انکڑے;
	html_content *html = (html_content *)انکڑے->c;
	بنية box *box = انکڑے->box;
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	html_drag_type drag_type;
	اتحاد html_drag_owner drag_owner;

	بدل(scrollbar_انکڑے->پیغام) {
	حالة SCROLLBAR_پیغام_MOVED:

		إذا (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			اكسر;
		}

		html__redraw_a_box(html, box);
		اكسر;
	حالة SCROLLBAR_پیغام_SCROLL_شروع:
	{
		بنية rect rect = {
			.x0 = scrollbar_انکڑے->x0,
			.y0 = scrollbar_انکڑے->y0,
			.x1 = scrollbar_انکڑے->x1,
			.y1 = scrollbar_انکڑے->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_انکڑے->scrollbar;
		html_سیٹ_drag_type(html, drag_type, drag_owner, &rect);
	}
		اكسر;
	حالة SCROLLBAR_پیغام_SCROLL_مکمل:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_سیٹ_drag_type(html, drag_type, drag_owner, عدم);

		پیغام_انکڑے.اشارہ_گر = BROWSER_اشارہ_گر_AUTO;
		content_نشر(انکڑے->c, CONTENT_پیغام_اشارہ_گر, &پیغام_انکڑے);
		اكسر;
	}
}


/* Documented in html_internal.h */
فراغ html_سیٹ_drag_type(html_content *html, html_drag_type drag_type,
		اتحاد html_drag_owner drag_owner, ثابت بنية rect *rect)
{
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;

	أكد(html != عدم);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	بدل (drag_type) {
	حالة HTML_DRAG_NONE:
		أكد(drag_owner.no_owner == true);
		پیغام_انکڑے.drag.type = CONTENT_DRAG_NONE;
		اكسر;

	حالة HTML_DRAG_SCROLLBAR:
	حالة HTML_DRAG_TEXTAREA_SCROLLBAR:
	حالة HTML_DRAG_CONTENT_SCROLL:
		پیغام_انکڑے.drag.type = CONTENT_DRAG_SCROLL;
		اكسر;

	حالة HTML_DRAG_SELECTION:
		أكد(drag_owner.no_owner == true);
		fallthrough;
	حالة HTML_DRAG_TEXTAREA_SELECTION:
	حالة HTML_DRAG_CONTENT_SELECTION:
		پیغام_انکڑے.drag.type = CONTENT_DRAG_SELECTION;
		اكسر;
	}
	پیغام_انکڑے.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_نشر((بنية content *)html, CONTENT_پیغام_DRAG, &پیغام_انکڑے);
}

/* Documented in html_internal.h */
فراغ html_سیٹ_focus(html_content *html, html_focus_type focus_type,
		اتحاد html_focus_owner focus_owner, bool hide_caret,
		صحيح x, صحيح y, صحيح اچائی, ثابت بنية rect *clip)
{
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	صحيح x_off = 0;
	صحيح y_off = 0;
	بنية rect cr;
	bool textarea_گواچا_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	أكد(html != عدم);

	بدل (focus_type) {
	حالة HTML_FOCUS_SELF:
		أكد(focus_owner.self == true);
		إذا (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			أرجع;
		اكسر;

	حالة HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		اكسر;

	حالة HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		اكسر;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	إذا (textarea_گواچا_focus) {
		پیغام_انکڑے.caret.type = CONTENT_CARET_REMOVE;
	} وإلا إذا (focus_type != HTML_FOCUS_SELF && hide_caret) {
		پیغام_انکڑے.caret.type = CONTENT_CARET_HIDE;
	} وإلا {
		إذا (clip != عدم) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		پیغام_انکڑے.caret.type = CONTENT_CARET_سیٹ_POS;
		پیغام_انکڑے.caret.pos.x = x + x_off;
		پیغام_انکڑے.caret.pos.y = y + y_off;
		پیغام_انکڑے.caret.pos.اچائی = اچائی;
		پیغام_انکڑے.caret.pos.clip = (clip == عدم) ? عدم : &cr;
	}

	/* Inform of the content's drag status change */
	content_نشر((بنية content *)html, CONTENT_پیغام_CARET, &پیغام_انکڑے);
}

/* Documented in html_internal.h */
فراغ html_سیٹ_selection(html_content *html, html_selection_type selection_type,
		اتحاد html_selection_owner selection_owner, bool read_only)
{
	اتحاد content_پیغام_انکڑے پیغام_انکڑے;
	بنية box *box;
	bool بدلیا = false;
	bool same_type = html->selection_type == selection_type;

	أكد(html != عدم);

	إذا ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		بدلیا = true;

	/* Clear any existing selection */
	إذا (html->selection_type != HTML_SELECTION_NONE) {
		بدل (html->selection_type) {
		حالة HTML_SELECTION_SELF:
			إذا (same_type)
				اكسر;
			selection_clear(html->sel, true);
			اكسر;
		حالة HTML_SELECTION_TEXTAREA:
			إذا (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				اكسر;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->انکڑے.متن.ta);
			اكسر;
		حالة HTML_SELECTION_CONTENT:
			إذا (same_type && html->selection_owner.content ==
					selection_owner.content)
				اكسر;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	إذا (!بدلیا)
		/* Don't need to report lack of change to owner */
		أرجع;

	/* Prepare msg */
	بدل (selection_type) {
	حالة HTML_SELECTION_NONE:
		أكد(selection_owner.none == true);
		پیغام_انکڑے.selection.selection = false;
		اكسر;
	حالة HTML_SELECTION_SELF:
		أكد(selection_owner.none == false);
		fallthrough;
	حالة HTML_SELECTION_TEXTAREA:
	حالة HTML_SELECTION_CONTENT:
		پیغام_انکڑے.selection.selection = true;
		اكسر;
	افتراضي:
		اكسر;
	}
	پیغام_انکڑے.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_نشر((بنية content *)html, CONTENT_پیغام_SELECTION,
			&پیغام_انکڑے);
}
