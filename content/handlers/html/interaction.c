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

ساكن browser_اشارو_shape get_اشارو_shape(بنية box *box, bool imagemap)
{
	browser_اشارو_shape اشارو;
	css_computed_style *style;
	تعداد css_cursor_e cursor;
	lwc_string **cursor_uris;

	إذا (box->type == BOX_FLOAT_کاٻو || box->type == BOX_FLOAT_ساڄو)
		style = box->children->style;
	وإلا
		style = box->style;

	إذا (style == عدم)
		أرجع BROWSER_اشارو_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	بدل (cursor) {
	حالة CSS_CURSOR_AUTO:
		إذا (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			اشارو = BROWSER_اشارو_نقطو;
		} وإلا إذا (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			اشارو = BROWSER_اشارو_CARET;
		} وإلا {
			/* html content doesn't mind */
			اشارو = BROWSER_اشارو_AUTO;
		}
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		اشارو = BROWSER_اشارو_CROSS;
		اكسر;
	حالة CSS_CURSOR_اشارو:
		اشارو = BROWSER_اشارو_نقطو;
		اكسر;
	حالة CSS_CURSOR_MOVE:
		اشارو = BROWSER_اشارو_MOVE;
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		اشارو = BROWSER_اشارو_ساڄو;
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		اشارو = BROWSER_اشارو_کاٻو;
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		اشارو = BROWSER_اشارو_مٿي;
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		اشارو = BROWSER_اشارو_هيٺ_ڏانهن;
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		اشارو = BROWSER_اشارو_RU;
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		اشارو = BROWSER_اشارو_LD;
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		اشارو = BROWSER_اشارو_RD;
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		اشارو = BROWSER_اشارو_LU;
		اكسر;
	حالة CSS_CURSOR_متن:
		اشارو = BROWSER_اشارو_CARET;
		اكسر;
	حالة CSS_CURSOR_WAIT:
		اشارو = BROWSER_اشارو_WAIT;
		اكسر;
	حالة CSS_CURSOR_ترقي:
		اشارو = BROWSER_اشارو_ترقي;
		اكسر;
	حالة CSS_CURSOR_HELP:
		اشارو = BROWSER_اشارو_HELP;
		اكسر;
	افتراضي:
		اشارو = BROWSER_اشارو_DEFAULT;
		اكسر;
	}

	أرجع اشارو;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ساكن فراغ html_box_drag_شروعات(بنية box *box, صحيح x, صحيح y)
{
	صحيح box_x, box_y;
	صحيح scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	إذا (box->scroll_x != عدم) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٽي_2] +
				box->اوچائي + box->padding[BOTTOM] -
				SCROLLBAR_ويڪر);
		scrollbar_شروعات_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} وإلا إذا (box->scroll_y != عدم) {
		scroll_mouse_x = x - (box_x + box->padding[کاٻو_2] +
				box->ويڪر + box->padding[ساڄو_2] -
				SCROLLBAR_ويڪر);
		scroll_mouse_y = y - box_y;

		scrollbar_شروعات_content_drag(box->scroll_y,
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
ساكن size_t html_selection_drag_پڇاڙي(بنية html_content *html,
		browser_mouse_state mouse, صحيح x, صحيح y, صحيح dir)
{
	صحيح pixel_وٿي;
	بنية box *box;
	صحيح dx, dy;
	size_t idx = 0;

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	إذا (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->متن, box->ڊيگهائي,
				       dx, &idx, &pixel_وٿي);

		idx += box->byte_وٿي;
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
html_image_coords_dom_user_معلومات_handler(dom_ڳنڍ_operation operation,
					 dom_string *ڪنجي,
					 فراغ *معلومات_2,
					 بنية dom_ڳنڍ *src,
					 بنية dom_ڳنڍ *dst)
{
	بنية image_داخلا_coords *oldcoords, *coords = معلومات_2, *newcoords;

	إذا (!dom_string_isequal(corestring_dom_ns_ڪنجي_image_coords_ڳنڍ_معلومات,
				ڪنجي) || coords == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_ڳنڍ_CLONED:
		newcoords = صفر_يادگيري_ڏيو(1, حجم(*newcoords));
		إذا (newcoords != عدم) {
			*newcoords = *coords;
			إذا (dom_ڳنڍ_سيٽ_user_معلومات(dst,
				 corestring_dom_ns_ڪنجي_image_coords_ڳنڍ_معلومات,
				 newcoords,
				 html_image_coords_dom_user_معلومات_handler,
				 &oldcoords) == DOM_NO_ERR) {
				حرر(oldcoords);
			}
		}
		اكسر;

	حالة DOM_ڳنڍ_DELETED:
		حرر(coords);
		اكسر;

	حالة DOM_ڳنڍ_RENAMED:
	حالة DOM_ڳنڍ_IMPORTED:
	حالة DOM_ڳنڍ_ADOPTED:
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
html_حد_کان_مٿي_scroll_drag_پڇاڙي(بنية scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      صحيح x, صحيح y)
{
	صحيح scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	بنية html_scrollbar_معلومات *معلومات = scrollbar_get_معلومات(scrollbar);
	بنية box *box;

	box = معلومات->box;
	box_coords(box, &box_x, &box_y);

	إذا (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[چوٽي_2] +
				box->اوچائي + box->padding[BOTTOM] -
				SCROLLBAR_ويڪر);
		scrollbar_mouse_drag_پڇاڙي(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} وإلا {
		scroll_mouse_x = x - (box_x + box->padding[کاٻو_2] +
				box->ويڪر + box->padding[ساڄو_2] -
				SCROLLBAR_ويڪر);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_پڇاڙي(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
ساكن nserror
mouse_action_select_menu(html_content *html,
			 بنية browser_دري *bw,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;
	ثابت حرف *حالت;
	صحيح ويڪر, اوچائي;
	بنية hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	أكد(html->visible_select_menu != عدم);

	bw_drag_type = browser_دري_get_drag_type(bw);
	إذا (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_پڇاڙي(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[کاٻو_2].ويڪر;
	box_y += box->اوچائي + box->border[BOTTOM].ويڪر +
		box->padding[BOTTOM] + box->padding[چوٽي_2];

	حالت = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	إذا (حالت != عدم) {
		/* set status if menu still open */
		اتحاد content_نياپو_معلومات نياپو_معلومات;
		نياپو_معلومات.explicit_حالت_متن = حالت;
		content_نشر((بنية content *)html,
				  CONTENT_نياپو_حالت,
				  &نياپو_معلومات);
		أرجع NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &ويڪر, &اوچائي);

	html->visible_select_menu = عدم;

	bw_content = browser_دري_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       ويڪر,
			       اوچائي);
	أرجع NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ساكن nserror
mouse_action_drag_selection(html_content *html,
			    بنية browser_دري *bw,
			    browser_mouse_state mouse,
			    صحيح x, صحيح y)
{
	بنية box *box;
	صحيح dir = -1;
	صحيح dx, dy;
	size_t idx;
	اتحاد html_drag_owner drag_owner;
	صحيح pixel_وٿي;
	plot_font_style_t fstyle;

	إذا (!mouse) {
		/* End of selection drag */
		إذا (selection_dragging_شروعات(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_پڇاڙي(html, mouse, x, y, dir);

		إذا (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_سيٽ_drag_type(html, HTML_DRAG_NONE, drag_owner, عدم);

		أرجع NSERROR_OK;
	}

	إذا (selection_dragging_شروعات(html->sel)) {
		dir = 1;
	}

	box = box_pick_متن_box(html, x, y, dir, &dx, &dy);
	إذا (box != عدم) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->متن,
				       box->ڊيگهائي,
				       dx,
				       &idx,
				       &pixel_وٿي);

		selection_track(html->sel, mouse, box->byte_وٿي + idx);
	}
	أرجع NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ساكن nserror
mouse_action_drag_scrollbar(html_content *html,
			    بنية browser_دري *bw,
			    browser_mouse_state mouse,
			    صحيح x, صحيح y)
{
	بنية scrollbar *scr;
	بنية html_scrollbar_معلومات *معلومات;
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;
	ثابت حرف *حالت;
	صحيح scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_حالت scrollbar_حالت;

	scr = html->drag_owner.scrollbar;

	إذا (!mouse) {
		/* drag end: scrollbar */
		html_حد_کان_مٿي_scroll_drag_پڇاڙي(scr, mouse, x, y);
	}

	معلومات = scrollbar_get_معلومات(scr);

	box = معلومات->box;

	box_coords(box, &box_x, &box_y);

	إذا (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[چوٽي_2] +
				      box->اوچائي + box->padding[BOTTOM] -
				      SCROLLBAR_ويڪر);
		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} وإلا {
		scroll_mouse_x = x - (box_x + box->padding[کاٻو_2] +
				      box->ويڪر + box->padding[ساڄو_2] -
				      SCROLLBAR_ويڪر);
		scroll_mouse_y = y - box_y;

		scrollbar_حالت = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	حالت = scrollbar_mouse_حالت_منزل_پيغام(scrollbar_حالت);

	إذا (حالت != عدم) {
		اتحاد content_نياپو_معلومات نياپو_معلومات;

		نياپو_معلومات.explicit_حالت_متن = حالت;
		content_نشر((بنية content *)html,
				  CONTENT_نياپو_حالت,
				  &نياپو_معلومات);
	}

	أرجع NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ساكن nserror
mouse_action_drag_textarea(html_content *html,
			    بنية browser_دري *bw,
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
	textarea_mouse_action(box->gadget->معلومات.متن.ta,
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
			  بنية browser_دري *bw,
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
		browser_اشارو_shape اشارو; /**< pointer shape */
		تعداد {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_پٺ, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} نتيجو;

	/** dom node */
	بنية dom_ڳنڍ *ڳنڍ;

	/** html object */
	بنية {
		بنية box *box;
		صحيح pos_x;
		صحيح pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	بنية browser_دري *iframe;

	/** link either from href or imagemap */
	بنية {
		بنية box *box;
		nsurl *url;
		ثابت حرف *هدف;
		bool is_imagemap;
	} link;

	/** gadget */
	بنية {
		بنية form_control *control;
		بنية box *box;
		صحيح box_x;
		صحيح box_y;
		ثابت حرف *هدف;
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
get_mouse_action_ڳنڍ(html_content *html,
		      صحيح x, صحيح y,
		      بنية mouse_action_state *man)
{
	بنية box *box;
	صحيح box_x = 0;
	صحيح box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, حجم(بنية mouse_action_state));
	man->ڳنڍ = html->layout->ڳنڍ; /* Default dom node to the <HTML> */
	man->نتيجو.اشارو = BROWSER_اشارو_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[کاٻو_2];
	box_y = box->margin[چوٽي_2];

	افعل {
		/* skip hidden boxes */
		إذا ((box->style != عدم) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			اذهب ايندڙ_box;
		}

		إذا (box->ڳنڍ != عدم) {
			man->ڳنڍ = box->ڳنڍ;
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
			man->link.هدف = box->هدف;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		إذا (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.هدف);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		إذا (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			إذا (box->gadget->form) {
				man->gadget.هدف = box->gadget->form->هدف;
			}
		}

		إذا (box->title) {
			man->title = box->title;
		}

		man->نتيجو.اشارو = get_اشارو_shape(box, false);

		إذا ((box->scroll_x != عدم) ||
		    (box->scroll_y != عدم)) {
			صحيح padding_کاٻو;
			صحيح padding_ساڄو;
			صحيح padding_چوٽي;
			صحيح padding_bottom;

			إذا (man->drag_candidate == عدم) {
				man->drag_candidate = box;
			}

			padding_کاٻو = box_x +
					scrollbar_get_وٿي(box->scroll_x);
			padding_ساڄو = padding_کاٻو + box->padding[کاٻو_2] +
					box->ويڪر + box->padding[ساڄو_2];
			padding_چوٽي = box_y +
					scrollbar_get_وٿي(box->scroll_y);
			padding_bottom = padding_چوٽي + box->padding[چوٽي_2] +
					box->اوچائي + box->padding[BOTTOM];

			إذا ((x > padding_کاٻو) &&
			    (x < padding_ساڄو) &&
			    (y > padding_چوٽي) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				إذا ((box->scroll_y != عدم) &&
				    (x > (padding_ساڄو - SCROLLBAR_ويڪر))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ساڄو - SCROLLBAR_ويڪر);
					man->scroll.mouse_y = y - padding_چوٽي;
					اكسر;

				} وإلا إذا ((box->scroll_x != عدم) &&
					   (y > (padding_bottom -
							SCROLLBAR_ويڪر))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_کاٻو;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_ويڪر);
					اكسر;
				}
			}
		}

		إذا (box->متن && !box->object) {
			man->متن.box = box;
			man->متن.box_x = box_x;
		}

	ايندڙ_box:
		/* iterate to next box */
		box = box_at_نقطو(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} طالما (box != عدم);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	أكد(man->ڳنڍ != عدم);

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
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	بدل (mas->gadget.control->type) {
	حالة GADGET_SELECT:
		mas->نتيجو.حالت = messages_get("FormSelect");
		mas->نتيجو.اشارو = BROWSER_اشارو_MENU;
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
			mas->نتيجو.اشارو = BROWSER_اشارو_DEFAULT;
		} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			نياپو_معلومات.select_menu.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_نياپو_SELECTMENU,
					  &نياپو_معلومات);
		}
		اكسر;

	حالة GADGET_CHECKBOX:
		mas->نتيجو.حالت = messages_get("FormCheckbox");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_داخلا_element_سيٽ_checked(
				(dom_html_داخلا_element *)(mas->gadget.control->ڳنڍ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		اكسر;

	حالة GADGET_RADIO:
		mas->نتيجو.حالت = messages_get("FormRadio");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_سيٽ(mas->gadget.control);
		}
		اكسر;

	حالة GADGET_IMAGE:
		/* This falls through to SUBMIT */
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			بنية image_داخلا_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = صفر_يادگيري_ڏيو(1, حجم(*coords));
			إذا (coords == عدم) {
				أرجع NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			إذا (dom_ڳنڍ_سيٽ_user_معلومات(
				mas->gadget.control->ڳنڍ,
				corestring_dom_ns_ڪنجي_image_coords_ڳنڍ_معلومات,
				coords,
				html_image_coords_dom_user_معلومات_handler,
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
			mas->نتيجو.حالت = حالت_بفر;
			mas->نتيجو.اشارو = get_اشارو_shape(mas->gadget.box,
								false);
			إذا (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->نتيجو.action = ACTION_SUBMIT;
			}
		} وإلا {
			mas->نتيجو.حالت = messages_get("FormBadSubmit");
		}
		اكسر;

	حالة GADGET_TEXTBOX:
	حالة GADGET_PASSWORD:
	حالة GADGET_TEXTAREA:
		إذا (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->نتيجو.حالت = messages_get("FormTextarea");
		} وإلا {
			mas->نتيجو.حالت = messages_get("FormTextbox");
		}

		إذا (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;
			html_سيٽ_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_حالت = textarea_mouse_action(mas->gadget.control->معلومات.متن.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		إذا (ta_حالت & TEXTAREA_MOUSE_EDITOR) {
			mas->نتيجو.اشارو = get_اشارو_shape(mas->gadget.box, false);
		} وإلا {
			mas->نتيجو.اشارو = BROWSER_اشارو_DEFAULT;
			mas->نتيجو.حالت = scrollbar_mouse_حالت_منزل_پيغام(ta_حالت >> 3);
		}
		اكسر;

	حالة GADGET_HIDDEN:
		/* not possible: no box generated */
		اكسر;

	حالة GADGET_RESET:
		mas->نتيجو.حالت = messages_get("FormReset");
		اكسر;

	حالة GADGET_FILE:
		mas->نتيجو.حالت = messages_get("FormFile");
		إذا (mouse & BROWSER_MOUSE_CLICK_1) {
			نياپو_معلومات.gadget_click.gadget = mas->gadget.control;
			content_نشر(c,
					  CONTENT_نياپو_GADGETCLICK,
					  &نياپو_معلومات);
		}
		اكسر;

	حالة GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->نتيجو.حالت = messages_get("FormButton");
		اكسر;
	}

	أرجع NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ساكن nserror
iframe_mouse_action(بنية browser_دري *bw,
		    browser_mouse_state mouse,
		    صحيح x, صحيح y,
		    بنية mouse_action_state *mas)
{
	صحيح pos_x, pos_y;
	عائم scale;

	scale = browser_دري_get_scale(bw);

	browser_دري_get_position(mas->iframe, false, &pos_x, &pos_y);

	إذا (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_دري_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} وإلا {
		browser_دري_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->نتيجو.action = ACTION_NOSEND;

	أرجع NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ساكن nserror
html_object_mouse_action(html_content *html,
			 بنية browser_دري *bw,
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
		html_سيٽ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
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

	mas->نتيجو.action = ACTION_NOSEND;
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
		  بنية browser_دري *bw,
		  browser_mouse_state mouse,
		  صحيح x, صحيح y,
		  بنية mouse_action_state *mas)
{
	nserror res;
	حرف *url_s = عدم;
	size_t url_l = 0;
	ساكن حرف حالت_بفر[200];
	اتحاد content_نياپو_معلومات نياپو_معلومات;

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

	mas->نتيجو.حالت = حالت_بفر;

	mas->نتيجو.اشارو = get_اشارو_shape(mas->link.box,
						mas->link.is_imagemap);

	إذا (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_دري_navigate(bw,
					mas->link.url,
					content_get_url((بنية content *)html),
					BW_NAVIGATE_DOWNLOAD,
					عدم,
					عدم,
					عدم);

	} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		نياپو_معلومات.savelink.url = mas->link.url;
		نياپو_معلومات.savelink.title = mas->title;
		content_نشر((بنية content *)html,
				  CONTENT_نياپو_SAVELINK,
				  &نياپو_معلومات);

	} وإلا إذا (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		إذا (is_javascript_navigate_url(mas->link.url)) {
			mas->نتيجو.action = ACTION_JS;
		} وإلا {
			mas->نتيجو.action = ACTION_NAVIGATE;
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
		html_سيٽ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, عدم);
	}

	أرجع NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ساكن nserror
default_mouse_action(html_content *html,
		  بنية browser_دري *bw,
		  browser_mouse_state mouse,
		  صحيح x, صحيح y,
		  بنية mouse_action_state *mas)
{
	بنية content *c = (بنية content *)html;

	/* frame resizing */
	إذا (browser_دري_چوکٺو_resize_شروعات(bw, mouse, x, y, &mas->نتيجو.اشارو)) {
		إذا (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->نتيجو.حالت = messages_get("FrameDrag");
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
		html_سيٽ_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, عدم);
	}
	إذا (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_سيٽ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	إذا (mas->متن.box) {
		صحيح pixel_وٿي;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->متن.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->متن.box->متن,
				       mas->متن.box->ڊيگهائي,
				       x - mas->متن.box_x,
				       &idx,
				       &pixel_وٿي);

		إذا (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->متن.box->byte_وٿي + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			اتحاد html_drag_owner drag_owner;

			إذا (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_سيٽ_drag_type(html,
						   drag_type,
						   drag_owner,
						   عدم);
				mas->نتيجو.حالت = messages_get("Selecting");
			}

			إذا (selection_سرگرم(html->sel)) {
				sel_owner.none = false;
				html_سيٽ_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} وإلا إذا (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_سيٽ_selection(html, HTML_SELECTION_NONE,
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
		html_سيٽ_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} وإلا إذا (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_سيٽ_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	إذا (mas->title) {
		mas->نتيجو.حالت = mas->title;
	}

	إذا (mouse & BROWSER_MOUSE_DRAG_1) {
		إذا (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_نياپو_معلومات نياپو_معلومات;
			نياپو_معلومات.dragsave.type = CONTENT_SAVE_COMPLETE;
			نياپو_معلومات.dragsave.content = عدم;
			content_نشر(c, CONTENT_نياپو_DRAGSAVE, &نياپو_معلومات);
		} وإلا {
			إذا (mas->drag_candidate == عدم) {
				browser_دري_صفحو_drag_شروعات(bw, x, y);
			} وإلا {
				html_box_drag_شروعات(mas->drag_candidate, x, y);
			}
			mas->نتيجو.اشارو = BROWSER_اشارو_MOVE;
		}
	} وإلا إذا (mouse & BROWSER_MOUSE_DRAG_2) {
		إذا (mouse & BROWSER_MOUSE_MOD_2) {
			اتحاد content_نياپو_معلومات نياپو_معلومات;
			نياپو_معلومات.dragsave.type = CONTENT_SAVE_ذريعو;
			نياپو_معلومات.dragsave.content = عدم;
			content_نشر(c, CONTENT_نياپو_DRAGSAVE, &نياپو_معلومات);
		} وإلا {
			إذا (mas->drag_candidate == عدم) {
				browser_دري_صفحو_drag_شروعات(bw, x, y);
			} وإلا {
				html_box_drag_شروعات(mas->drag_candidate, x, y);
			}
			mas->نتيجو.اشارو = BROWSER_اشارو_MOVE;
		}
	}


	أرجع default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ساكن nserror
mouse_action_drag_none(html_content *html,
		       بنية browser_دري *bw,
		       browser_mouse_state mouse,
		       صحيح x, صحيح y)
{
	nserror res;
	بنية content *c = (بنية content *)html;
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	lwc_string *رستو;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ساكن بنية mouse_action_state mas;

	res = get_mouse_action_ڳنڍ(html, x, y, &mas);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	إذا (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.نتيجو.action = ACTION_پٺ;
	} وإلا إذا (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.نتيجو.action = ACTION_FORWARD;
	} وإلا إذا (mas.scroll.bar) {
		mas.نتيجو.حالت = scrollbar_mouse_حالت_منزل_پيغام(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.نتيجو.اشارو = BROWSER_اشارو_DEFAULT;

	} وإلا إذا (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} وإلا إذا ((mas.object != عدم) && (mouse & BROWSER_MOUSE_MOD_2)) {

		إذا (mouse & BROWSER_MOUSE_DRAG_2) {
			نياپو_معلومات.dragsave.type = CONTENT_SAVE_NATIVE;
			نياپو_معلومات.dragsave.content = mas.object;
			content_نشر(c, CONTENT_نياپو_DRAGSAVE, &نياپو_معلومات);

		} وإلا إذا (mouse & BROWSER_MOUSE_DRAG_1) {
			نياپو_معلومات.dragsave.type = CONTENT_SAVE_ORIG;
			نياپو_معلومات.dragsave.content = mas.object;
			content_نشر(c, CONTENT_نياپو_DRAGSAVE, &نياپو_معلومات);
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
	إذا (mas.نتيجو.action != ACTION_NOSEND) {
		نياپو_معلومات.explicit_حالت_متن = mas.نتيجو.حالت;
		content_نشر(c, CONTENT_نياپو_حالت, &نياپو_معلومات);

		نياپو_معلومات.اشارو = mas.نتيجو.اشارو;
		content_نشر(c, CONTENT_نياپو_اشارو, &نياپو_معلومات);
	}

	/* fire dom click event */
	إذا (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ڳنڍ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	بدل (mas.نتيجو.action) {
	حالة ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_دري_ڳوليو_هدف(bw,
							     mas.gadget.هدف,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		اكسر;

	حالة ACTION_NAVIGATE:
		res = browser_دري_navigate(
				browser_دري_ڳوليو_هدف(bw,
							   mas.link.هدف,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				عدم,
				عدم,
				عدم);
		اكسر;

	حالة ACTION_JS:
		رستو = nsurl_get_component(mas.link.url, NSURL_رستو);
		إذا (رستو != عدم) {
			html_exec(c,
				  lwc_string_معلومات(رستو),
				  lwc_string_ڊيگهائي(رستو));
			lwc_string_unref(رستو);
		}
		اكسر;

	حالة ACTION_پٺ:
		res = browser_دري_history_پٺ(bw, false);
		اكسر;

	حالة ACTION_FORWARD:
		res = browser_دري_history_forward(bw, false);
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
			 بنية browser_دري *bw,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	أرجع html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(بنية content *c,
		  بنية browser_دري *bw,
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
bool html_keypress(بنية content *c, uint32_t ڪنجي)
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
	إذا (html->layout != عدم && html->layout->ڳنڍ != عدم) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ڳنڍ, true, true, ڪنجي);
	}

	بدل (html->focus_type) {
	حالة HTML_FOCUS_CONTENT:
		أرجع content_keypress(html->focus_owner.content->object, ڪنجي);

	حالة HTML_FOCUS_TEXTAREA:
		إذا (box_textarea_keypress(html, html->focus_owner.textarea, ڪنجي) == NSERROR_OK) {
			أرجع true;
		} وإلا {
			أرجع false;
		}

	افتراضي:
		/* Deal with it below */
		اكسر;
	}

	بدل (ڪنجي) {
	حالة NS_ڪنجي_COPY_SELECTION:
		selection_copy_منزل_clipboard(sel);
		أرجع true;

	حالة NS_ڪنجي_CLEAR_SELECTION:
		selection_clear(sel, true);
		أرجع true;

	حالة NS_ڪنجي_SELECT_ALL:
		selection_select_all(sel);
		أرجع true;

	حالة NS_ڪنجي_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		أرجع selection_clear(sel, true);
	}

	أرجع false;
}


/**
 * Callback for in-page scrollbars.
 */
فراغ html_حد_کان_مٿي_scroll_callback(فراغ *client_معلومات,
		بنية scrollbar_نياپو_معلومات *scrollbar_معلومات)
{
	بنية html_scrollbar_معلومات *معلومات = client_معلومات;
	html_content *html = (html_content *)معلومات->c;
	بنية box *box = معلومات->box;
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	html_drag_type drag_type;
	اتحاد html_drag_owner drag_owner;

	بدل(scrollbar_معلومات->نياپو) {
	حالة SCROLLBAR_نياپو_MOVED:

		إذا (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			اكسر;
		}

		html__redraw_a_box(html, box);
		اكسر;
	حالة SCROLLBAR_نياپو_SCROLL_شروعات:
	{
		بنية rect rect = {
			.x0 = scrollbar_معلومات->x0,
			.y0 = scrollbar_معلومات->y0,
			.x1 = scrollbar_معلومات->x1,
			.y1 = scrollbar_معلومات->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_معلومات->scrollbar;
		html_سيٽ_drag_type(html, drag_type, drag_owner, &rect);
	}
		اكسر;
	حالة SCROLLBAR_نياپو_SCROLL_ختم:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_سيٽ_drag_type(html, drag_type, drag_owner, عدم);

		نياپو_معلومات.اشارو = BROWSER_اشارو_AUTO;
		content_نشر(معلومات->c, CONTENT_نياپو_اشارو, &نياپو_معلومات);
		اكسر;
	}
}


/* Documented in html_internal.h */
فراغ html_سيٽ_drag_type(html_content *html, html_drag_type drag_type,
		اتحاد html_drag_owner drag_owner, ثابت بنية rect *rect)
{
	اتحاد content_نياپو_معلومات نياپو_معلومات;

	أكد(html != عدم);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	بدل (drag_type) {
	حالة HTML_DRAG_NONE:
		أكد(drag_owner.no_owner == true);
		نياپو_معلومات.drag.type = CONTENT_DRAG_NONE;
		اكسر;

	حالة HTML_DRAG_SCROLLBAR:
	حالة HTML_DRAG_TEXTAREA_SCROLLBAR:
	حالة HTML_DRAG_CONTENT_SCROLL:
		نياپو_معلومات.drag.type = CONTENT_DRAG_SCROLL;
		اكسر;

	حالة HTML_DRAG_SELECTION:
		أكد(drag_owner.no_owner == true);
		fallthrough;
	حالة HTML_DRAG_TEXTAREA_SELECTION:
	حالة HTML_DRAG_CONTENT_SELECTION:
		نياپو_معلومات.drag.type = CONTENT_DRAG_SELECTION;
		اكسر;
	}
	نياپو_معلومات.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_نشر((بنية content *)html, CONTENT_نياپو_DRAG, &نياپو_معلومات);
}

/* Documented in html_internal.h */
فراغ html_سيٽ_focus(html_content *html, html_focus_type focus_type,
		اتحاد html_focus_owner focus_owner, bool hide_caret,
		صحيح x, صحيح y, صحيح اوچائي, ثابت بنية rect *clip)
{
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	صحيح x_off = 0;
	صحيح y_off = 0;
	بنية rect cr;
	bool textarea_گم_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
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

	إذا (textarea_گم_focus) {
		نياپو_معلومات.caret.type = CONTENT_CARET_REMOVE;
	} وإلا إذا (focus_type != HTML_FOCUS_SELF && hide_caret) {
		نياپو_معلومات.caret.type = CONTENT_CARET_HIDE;
	} وإلا {
		إذا (clip != عدم) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		نياپو_معلومات.caret.type = CONTENT_CARET_سيٽ_POS;
		نياپو_معلومات.caret.pos.x = x + x_off;
		نياپو_معلومات.caret.pos.y = y + y_off;
		نياپو_معلومات.caret.pos.اوچائي = اوچائي;
		نياپو_معلومات.caret.pos.clip = (clip == عدم) ? عدم : &cr;
	}

	/* Inform of the content's drag status change */
	content_نشر((بنية content *)html, CONTENT_نياپو_CARET, &نياپو_معلومات);
}

/* Documented in html_internal.h */
فراغ html_سيٽ_selection(html_content *html, html_selection_type selection_type,
		اتحاد html_selection_owner selection_owner, bool read_only)
{
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	بنية box *box;
	bool بدليو = false;
	bool same_type = html->selection_type == selection_type;

	أكد(html != عدم);

	إذا ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		بدليو = true;

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
			textarea_clear_selection(box->gadget->معلومات.متن.ta);
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

	إذا (!بدليو)
		/* Don't need to report lack of change to owner */
		أرجع;

	/* Prepare msg */
	بدل (selection_type) {
	حالة HTML_SELECTION_NONE:
		أكد(selection_owner.none == true);
		نياپو_معلومات.selection.selection = false;
		اكسر;
	حالة HTML_SELECTION_SELF:
		أكد(selection_owner.none == false);
		fallthrough;
	حالة HTML_SELECTION_TEXTAREA:
	حالة HTML_SELECTION_CONTENT:
		نياپو_معلومات.selection.selection = true;
		اكسر;
	افتراضي:
		اكسر;
	}
	نياپو_معلومات.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_نشر((بنية content *)html, CONTENT_نياپو_SELECTION,
			&نياپو_معلومات);
}
