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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/mouse.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "netsurf/layout.h"
#അന്തര്ഭാവ "netsurf/keypress.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "content/textsearch.h"
#അന്തര്ഭാവ "desktop/browser_history.h"
#അന്തര്ഭാവ "desktop/frames.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/selection.h"
#അന്തര്ഭാവ "desktop/textarea.h"
#അന്തര്ഭാവ "javascript/js.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_textarea.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/imagemap.h"
#അന്തര്ഭാവ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

സ്ഥിതം browser_pointer_shape get_pointer_shape(ഘടന box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	എണ്ണൽ css_cursor_e cursor;
	lwc_string **cursor_uris;

	എങ്കിൽ (box->type == BOX_FLOAT_ഇടത് || box->type == BOX_FLOAT_വലത്)
		style = box->children->style;
	അല്ലെങ്കിൽ
		style = box->style;

	എങ്കിൽ (style == ശൂന്യ)
		തിരികെ BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	തിരഞ്ഞെടുപ്പ് (cursor) {
	അവസ്ഥ CSS_CURSOR_AUTO:
		എങ്കിൽ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} അല്ലെങ്കിൽ {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_വലത്;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_ഇടത്;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		നിർത്തുക;
	അവസ്ഥ CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		pointer = BROWSER_POINTER_DEFAULT;
		നിർത്തുക;
	}

	തിരികെ pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

സ്ഥിതം ശൂന്യം html_box_drag_start(ഘടന box *box, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	പൂർണ്ണസംഖ്യ box_x, box_y;
	പൂർണ്ണസംഖ്യ scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	എങ്കിൽ (box->scroll_x != ശൂന്യ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[മുകൾ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} അല്ലെങ്കിൽ എങ്കിൽ (box->scroll_y != ശൂന്യ) {
		scroll_mouse_x = x - (box_x + box->padding[ഇടത്_2] +
				box->width + box->padding[വലത്_2] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_start_content_drag(box->scroll_y,
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
സ്ഥിതം size_t html_selection_drag_end(ഘടന html_content *html,
		browser_mouse_state mouse, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ dir)
{
	പൂർണ്ണസംഖ്യ pixel_offset;
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	എങ്കിൽ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	തിരികെ idx;
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
സ്ഥിതം ശൂന്യം
html__image_coords_dom_user_data_handler(dom_കണ്ണി_operation operation,
					 dom_string *താക്കോൽ,
					 ശൂന്യം *_data,
					 ഘടന dom_കണ്ണി *src,
					 ഘടന dom_കണ്ണി *dst)
{
	ഘടന image_നിവേശം_coords *oldcoords, *coords = _data, *newcoords;

	എങ്കിൽ (!dom_string_isequal(corestring_dom_ns_താക്കോൽ_image_coords_കണ്ണി_data,
				താക്കോൽ) || coords == ശൂന്യ) {
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ് (operation) {
	അവസ്ഥ DOM_കണ്ണി_CLONED:
		newcoords = calloc(1, വലുപ്പം(*newcoords));
		എങ്കിൽ (newcoords != ശൂന്യ) {
			*newcoords = *coords;
			എങ്കിൽ (dom_കണ്ണി_set_user_data(dst,
				 corestring_dom_ns_താക്കോൽ_image_coords_കണ്ണി_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				സ്വതന്ത്രം(oldcoords);
			}
		}
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_DELETED:
		സ്വതന്ത്രം(coords);
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_RENAMED:
	അവസ്ഥ DOM_കണ്ണി_IMPORTED:
	അവസ്ഥ DOM_കണ്ണി_ADOPTED:
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		പ്രതിജ്ഞാ(0);
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
സ്ഥിതം ശൂന്യം
html_overflow_scroll_drag_end(ഘടന scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	പൂർണ്ണസംഖ്യ scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ഘടന html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	ഘടന box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	എങ്കിൽ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[മുകൾ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} അല്ലെങ്കിൽ {
		scroll_mouse_x = x - (box_x + box->padding[ഇടത്_2] +
				box->width + box->padding[വലത്_2] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
സ്ഥിതം nserror
mouse_action_select_menu(html_content *html,
			 ഘടന browser_window *bw,
			 browser_mouse_state mouse,
			 പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ box_x = 0;
	പൂർണ്ണസംഖ്യ box_y = 0;
	സ്ഥിരം അക്ഷരം *status;
	പൂർണ്ണസംഖ്യ width, height;
	ഘടന hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	പ്രതിജ്ഞാ(html->visible_select_menu != ശൂന്യ);

	bw_drag_type = browser_window_get_drag_type(bw);
	എങ്കിൽ (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ഇടത്_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[മുകൾ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	എങ്കിൽ (status != ശൂന്യ) {
		/* set status if menu still open */
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;
		സന്ദേശം_data.explicit_status_text = status;
		content_broadcast((ഘടന content *)html,
				  CONTENT_സന്ദേശം_STATUS,
				  &സന്ദേശം_data);
		തിരികെ NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = ശൂന്യ;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	തിരികെ NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
സ്ഥിതം nserror
mouse_action_drag_selection(html_content *html,
			    ഘടന browser_window *bw,
			    browser_mouse_state mouse,
			    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ dir = -1;
	പൂർണ്ണസംഖ്യ dx, dy;
	size_t idx;
	സംഘം html_drag_owner drag_owner;
	പൂർണ്ണസംഖ്യ pixel_offset;
	plot_font_style_t fstyle;

	എങ്കിൽ (!mouse) {
		/* End of selection drag */
		എങ്കിൽ (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		എങ്കിൽ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, ശൂന്യ);

		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	എങ്കിൽ (box != ശൂന്യ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	തിരികെ NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
സ്ഥിതം nserror
mouse_action_drag_scrollbar(html_content *html,
			    ഘടന browser_window *bw,
			    browser_mouse_state mouse,
			    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന scrollbar *scr;
	ഘടന html_scrollbar_data *data;
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ box_x = 0;
	പൂർണ്ണസംഖ്യ box_y = 0;
	സ്ഥിരം അക്ഷരം *status;
	പൂർണ്ണസംഖ്യ scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	എങ്കിൽ (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	എങ്കിൽ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[മുകൾ_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} അല്ലെങ്കിൽ {
		scroll_mouse_x = x - (box_x + box->padding[ഇടത്_2] +
				      box->width + box->padding[വലത്_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	എങ്കിൽ (status != ശൂന്യ) {
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;

		സന്ദേശം_data.explicit_status_text = status;
		content_broadcast((ഘടന content *)html,
				  CONTENT_സന്ദേശം_STATUS,
				  &സന്ദേശം_data);
	}

	തിരികെ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
സ്ഥിതം nserror
mouse_action_drag_textarea(html_content *html,
			    ഘടന browser_window *bw,
			    browser_mouse_state mouse,
			    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ box_x = 0;
	പൂർണ്ണസംഖ്യ box_y = 0;

	box = html->drag_owner.textarea;

	പ്രതിജ്ഞാ(box->gadget != ശൂന്യ);
	പ്രതിജ്ഞാ(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	തിരികെ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
സ്ഥിതം nserror
mouse_action_drag_content(html_content *html,
			  ഘടന browser_window *bw,
			  browser_mouse_state mouse,
			  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ box_x = 0;
	പൂർണ്ണസംഖ്യ box_y = 0;

	box = html->drag_owner.content;
	പ്രതിജ്ഞാ(box->object != ശൂന്യ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	തിരികെ NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ഘടന mouse_action_state {
	ഘടന {
		സ്ഥിരം അക്ഷരം *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		എണ്ണൽ {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_BACK, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} result;

	/** dom node */
	ഘടന dom_കണ്ണി *കണ്ണി;

	/** html object */
	ഘടന {
		ഘടന box *box;
		പൂർണ്ണസംഖ്യ pos_x;
		പൂർണ്ണസംഖ്യ pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ഘടന browser_window *iframe;

	/** link either from href or imagemap */
	ഘടന {
		ഘടന box *box;
		nsurl *url;
		സ്ഥിരം അക്ഷരം *target;
		bool is_imagemap;
	} link;

	/** gadget */
	ഘടന {
		ഘടന form_control *control;
		ഘടന box *box;
		പൂർണ്ണസംഖ്യ box_x;
		പൂർണ്ണസംഖ്യ box_y;
		സ്ഥിരം അക്ഷരം *target;
	} gadget;

	/** title */
	സ്ഥിരം അക്ഷരം *title;

	/** candidate box for drag operation */
	ഘടന box *drag_candidate;

	/** scrollbar */
	ഘടന {
		ഘടന scrollbar *bar;
		പൂർണ്ണസംഖ്യ mouse_x;
		പൂർണ്ണസംഖ്യ mouse_y;
	} scroll;

	/** text in box */
	ഘടന {
		ഘടന box *box;
		പൂർണ്ണസംഖ്യ box_x;
	} text;
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
സ്ഥിതം nserror
get_mouse_action_കണ്ണി(html_content *html,
		      പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		      ഘടന mouse_action_state *man)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ box_x = 0;
	പൂർണ്ണസംഖ്യ box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, വലുപ്പം(ഘടന mouse_action_state));
	man->കണ്ണി = html->layout->കണ്ണി; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ഇടത്_2];
	box_y = box->margin[മുകൾ_2];

	ചെയ്യുക {
		/* skip hidden boxes */
		എങ്കിൽ ((box->style != ശൂന്യ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			പോകുക next_box;
		}

		എങ്കിൽ (box->കണ്ണി != ശൂന്യ) {
			man->കണ്ണി = box->കണ്ണി;
		}

		എങ്കിൽ (box->object) {
			എങ്കിൽ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} അല്ലെങ്കിൽ {
				man->object = box->object;
			}
		}

		എങ്കിൽ (box->iframe) {
			man->iframe = box->iframe;
		}

		എങ്കിൽ (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		എങ്കിൽ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		എങ്കിൽ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			എങ്കിൽ (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		എങ്കിൽ (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		എങ്കിൽ ((box->scroll_x != ശൂന്യ) ||
		    (box->scroll_y != ശൂന്യ)) {
			പൂർണ്ണസംഖ്യ padding_ഇടത്;
			പൂർണ്ണസംഖ്യ padding_വലത്;
			പൂർണ്ണസംഖ്യ padding_മുകൾ;
			പൂർണ്ണസംഖ്യ padding_bottom;

			എങ്കിൽ (man->drag_candidate == ശൂന്യ) {
				man->drag_candidate = box;
			}

			padding_ഇടത് = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_വലത് = padding_ഇടത് + box->padding[ഇടത്_2] +
					box->width + box->padding[വലത്_2];
			padding_മുകൾ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_മുകൾ + box->padding[മുകൾ_2] +
					box->height + box->padding[BOTTOM];

			എങ്കിൽ ((x > padding_ഇടത്) &&
			    (x < padding_വലത്) &&
			    (y > padding_മുകൾ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				എങ്കിൽ ((box->scroll_y != ശൂന്യ) &&
				    (x > (padding_വലത് - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_വലത് - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_മുകൾ;
					നിർത്തുക;

				} അല്ലെങ്കിൽ എങ്കിൽ ((box->scroll_x != ശൂന്യ) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ഇടത്;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					നിർത്തുക;
				}
			}
		}

		എങ്കിൽ (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ആകുമ്പോൾ (box != ശൂന്യ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	പ്രതിജ്ഞാ(man->കണ്ണി != ശൂന്യ);

	തിരികെ NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
സ്ഥിതം nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		    ഘടന mouse_action_state *mas)
{
	ഘടന content *c = (ഘടന content *)html;
	textarea_mouse_status ta_status;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	തിരഞ്ഞെടുപ്പ് (mas->gadget.control->type) {
	അവസ്ഥ GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			എങ്കിൽ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ശൂന്യ;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
			സന്ദേശം_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_സന്ദേശം_SELECTMENU,
					  &സന്ദേശം_data);
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_നിവേശം_element_set_checked(
				(dom_html_നിവേശം_element *)(mas->gadget.control->കണ്ണി),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
			ഘടന image_നിവേശം_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, വലുപ്പം(*coords));
			എങ്കിൽ (coords == ശൂന്യ) {
				തിരികെ NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			എങ്കിൽ (dom_കണ്ണി_set_user_data(
				mas->gadget.control->കണ്ണി,
				corestring_dom_ns_താക്കോൽ_image_coords_കണ്ണി_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				തിരികെ NSERROR_OK;
			}
			സ്വതന്ത്രം(oldcoords);
		}
		fallthrough;

	അവസ്ഥ GADGET_SUBMIT:
		എങ്കിൽ (mas->gadget.control->form) {
			സ്ഥിതം അക്ഷരം status_buffer[200];

			snprintf(status_buffer,
				 വലുപ്പം status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			എങ്കിൽ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} അല്ലെങ്കിൽ {
			mas->result.status = messages_get("FormBadSubmit");
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_TEXTBOX:
	അവസ്ഥ GADGET_PASSWORD:
	അവസ്ഥ GADGET_TEXTAREA:
		എങ്കിൽ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} അല്ലെങ്കിൽ {
			mas->result.status = messages_get("FormTextbox");
		}

		എങ്കിൽ (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			സംഘം html_selection_owner sel_owner;
			sel_owner.none = true;
			html_set_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_status = textarea_mouse_action(mas->gadget.control->data.text.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		എങ്കിൽ (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} അല്ലെങ്കിൽ {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_HIDDEN:
		/* not possible: no box generated */
		നിർത്തുക;

	അവസ്ഥ GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		നിർത്തുക;

	അവസ്ഥ GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
			സന്ദേശം_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_സന്ദേശം_GADGETCLICK,
					  &സന്ദേശം_data);
		}
		നിർത്തുക;

	അവസ്ഥ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		നിർത്തുക;
	}

	തിരികെ NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
സ്ഥിതം nserror
iframe_mouse_action(ഘടന browser_window *bw,
		    browser_mouse_state mouse,
		    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		    ഘടന mouse_action_state *mas)
{
	പൂർണ്ണസംഖ്യ pos_x, pos_y;
	ദശാംശം scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} അല്ലെങ്കിൽ {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	തിരികെ NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
സ്ഥിതം nserror
html_object_mouse_action(html_content *html,
			 ഘടന browser_window *bw,
			 browser_mouse_state mouse,
			 പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			 ഘടന mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	എങ്കിൽ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		സംഘം html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} അല്ലെങ്കിൽ {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	തിരികെ NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
സ്ഥിതം bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	എങ്കിൽ (scheme != ശൂന്യ) {
		എങ്കിൽ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	തിരികെ is_js;
}


/**
 * process mouse activity on a link
 */
സ്ഥിതം nserror
link_mouse_action(html_content *html,
		  ഘടന browser_window *bw,
		  browser_mouse_state mouse,
		  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		  ഘടന mouse_action_state *mas)
{
	nserror res;
	അക്ഷരം *url_s = ശൂന്യ;
	size_t url_l = 0;
	സ്ഥിതം അക്ഷരം status_buffer[200];
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;

	എങ്കിൽ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		എങ്കിൽ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ശൂന്യ;
		}
	}

	എങ്കിൽ (mas->title) {
		snprintf(status_buffer,
			 വലുപ്പം status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} അല്ലെങ്കിൽ {
		snprintf(status_buffer,
			 വലുപ്പം status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	എങ്കിൽ (url_s != ശൂന്യ) {
		സ്വതന്ത്രം(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((ഘടന content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ശൂന്യ,
					ശൂന്യ,
					ശൂന്യ);

	} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		സന്ദേശം_data.savelink.url = mas->link.url;
		സന്ദേശം_data.savelink.title = mas->title;
		content_broadcast((ഘടന content *)html,
				  CONTENT_സന്ദേശം_SAVELINK,
				  &സന്ദേശം_data);

	} അല്ലെങ്കിൽ എങ്കിൽ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		എങ്കിൽ (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} അല്ലെങ്കിൽ {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	തിരികെ NSERROR_OK;
}


സ്ഥിതം nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	എങ്കിൽ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		സംഘം html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ശൂന്യ);
	}

	തിരികെ NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
സ്ഥിതം nserror
default_mouse_action(html_content *html,
		  ഘടന browser_window *bw,
		  browser_mouse_state mouse,
		  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		  ഘടന mouse_action_state *mas)
{
	ഘടന content *c = (ഘടന content *)html;

	/* frame resizing */
	എങ്കിൽ (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		എങ്കിൽ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		തിരികെ default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	സംഘം html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	എങ്കിൽ (click && html->focus_type != HTML_FOCUS_SELF) {
		സംഘം html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ശൂന്യ);
	}
	എങ്കിൽ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	എങ്കിൽ (mas->text.box) {
		പൂർണ്ണസംഖ്യ pixel_offset;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->text.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->text.box->text,
				       mas->text.box->length,
				       x - mas->text.box_x,
				       &idx,
				       &pixel_offset);

		എങ്കിൽ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			സംഘം html_drag_owner drag_owner;

			എങ്കിൽ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   ശൂന്യ);
				mas->result.status = messages_get("Selecting");
			}

			എങ്കിൽ (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} അല്ലെങ്കിൽ എങ്കിൽ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			തിരികെ default_mouse_action_focus(html, mouse);
		}

	} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	എങ്കിൽ (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} അല്ലെങ്കിൽ എങ്കിൽ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	എങ്കിൽ (mas->title) {
		mas->result.status = mas->title;
	}

	എങ്കിൽ (mouse & BROWSER_MOUSE_DRAG_1) {
		എങ്കിൽ (mouse & BROWSER_MOUSE_MOD_2) {
			സംഘം content_സന്ദേശം_data സന്ദേശം_data;
			സന്ദേശം_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			സന്ദേശം_data.dragsave.content = ശൂന്യ;
			content_broadcast(c, CONTENT_സന്ദേശം_DRAGSAVE, &സന്ദേശം_data);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (mas->drag_candidate == ശൂന്യ) {
				browser_window_page_drag_start(bw, x, y);
			} അല്ലെങ്കിൽ {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_DRAG_2) {
		എങ്കിൽ (mouse & BROWSER_MOUSE_MOD_2) {
			സംഘം content_സന്ദേശം_data സന്ദേശം_data;
			സന്ദേശം_data.dragsave.type = CONTENT_SAVE_SOURCE;
			സന്ദേശം_data.dragsave.content = ശൂന്യ;
			content_broadcast(c, CONTENT_സന്ദേശം_DRAGSAVE, &സന്ദേശം_data);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (mas->drag_candidate == ശൂന്യ) {
				browser_window_page_drag_start(bw, x, y);
			} അല്ലെങ്കിൽ {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	തിരികെ default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
സ്ഥിതം nserror
mouse_action_drag_none(html_content *html,
		       ഘടന browser_window *bw,
		       browser_mouse_state mouse,
		       പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	nserror res;
	ഘടന content *c = (ഘടന content *)html;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	സ്ഥിതം ഘടന mouse_action_state mas;

	res = get_mouse_action_കണ്ണി(html, x, y, &mas);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} അല്ലെങ്കിൽ എങ്കിൽ (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} അല്ലെങ്കിൽ എങ്കിൽ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} അല്ലെങ്കിൽ എങ്കിൽ ((mas.object != ശൂന്യ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		എങ്കിൽ (mouse & BROWSER_MOUSE_DRAG_2) {
			സന്ദേശം_data.dragsave.type = CONTENT_SAVE_NATIVE;
			സന്ദേശം_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_സന്ദേശം_DRAGSAVE, &സന്ദേശം_data);

		} അല്ലെങ്കിൽ എങ്കിൽ (mouse & BROWSER_MOUSE_DRAG_1) {
			സന്ദേശം_data.dragsave.type = CONTENT_SAVE_ORIG;
			സന്ദേശം_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_സന്ദേശം_DRAGSAVE, &സന്ദേശം_data);
		}

		/* \todo should have a drag-saving object msg */

	} അല്ലെങ്കിൽ എങ്കിൽ (mas.iframe != ശൂന്യ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} അല്ലെങ്കിൽ എങ്കിൽ (mas.html_object.box != ശൂന്യ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} അല്ലെങ്കിൽ എങ്കിൽ (mas.link.url != ശൂന്യ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} അല്ലെങ്കിൽ {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	/* send status and pointer message */
	എങ്കിൽ (mas.result.action != ACTION_NOSEND) {
		സന്ദേശം_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_സന്ദേശം_STATUS, &സന്ദേശം_data);

		സന്ദേശം_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_സന്ദേശം_POINTER, &സന്ദേശം_data);
	}

	/* fire dom click event */
	എങ്കിൽ (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.കണ്ണി, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	തിരഞ്ഞെടുപ്പ് (mas.result.action) {
	അവസ്ഥ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		നിർത്തുക;

	അവസ്ഥ ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ശൂന്യ,
				ശൂന്യ,
				ശൂന്യ);
		നിർത്തുക;

	അവസ്ഥ ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		എങ്കിൽ (path != ശൂന്യ) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		നിർത്തുക;

	അവസ്ഥ ACTION_BACK:
		res = browser_window_history_back(bw, false);
		നിർത്തുക;

	അവസ്ഥ ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		നിർത്തുക;

	അവസ്ഥ ACTION_NOSEND:
	അവസ്ഥ ACTION_NONE:
		res = NSERROR_OK;
		നിർത്തുക;
	}

	തിരികെ res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ഘടന content *c,
			 ഘടന browser_window *bw,
			 browser_mouse_state mouse,
			 പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	തിരികെ html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ഘടന content *c,
		  ഘടന browser_window *bw,
		  browser_mouse_state mouse,
		  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	എങ്കിൽ (html->visible_select_menu != ശൂന്യ) {
		തിരികെ mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	തിരഞ്ഞെടുപ്പ് (html->drag_type) {
	അവസ്ഥ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_TEXTAREA_SELECTION:
	അവസ്ഥ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_CONTENT_SELECTION:
	അവസ്ഥ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		/* Unknown content related drag type */
		പ്രതിജ്ഞാ(0 && "Unknown content related drag type");
	}

	എങ്കിൽ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	തിരികെ res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ഘടന content *c, uint32_t താക്കോൽ)
{
	html_content *html = (html_content *) c;
	ഘടന selection *sel = html->sel;

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
	എങ്കിൽ (html->layout != ശൂന്യ && html->layout->കണ്ണി != ശൂന്യ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->കണ്ണി, true, true, താക്കോൽ);
	}

	തിരഞ്ഞെടുപ്പ് (html->focus_type) {
	അവസ്ഥ HTML_FOCUS_CONTENT:
		തിരികെ content_keypress(html->focus_owner.content->object, താക്കോൽ);

	അവസ്ഥ HTML_FOCUS_TEXTAREA:
		എങ്കിൽ (box_textarea_keypress(html, html->focus_owner.textarea, താക്കോൽ) == NSERROR_OK) {
			തിരികെ true;
		} അല്ലെങ്കിൽ {
			തിരികെ false;
		}

	സ്ഥിരസ്ഥിതി:
		/* Deal with it below */
		നിർത്തുക;
	}

	തിരഞ്ഞെടുപ്പ് (താക്കോൽ) {
	അവസ്ഥ NS_താക്കോൽ_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_CLEAR_SELECTION:
		selection_clear(sel, true);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_SELECT_ALL:
		selection_select_all(sel);
		തിരികെ true;

	അവസ്ഥ NS_താക്കോൽ_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		തിരികെ selection_clear(sel, true);
	}

	തിരികെ false;
}


/**
 * Callback for in-page scrollbars.
 */
ശൂന്യം html_overflow_scroll_callback(ശൂന്യം *client_data,
		ഘടന scrollbar_സന്ദേശം_data *scrollbar_data)
{
	ഘടന html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	ഘടന box *box = data->box;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	html_drag_type drag_type;
	സംഘം html_drag_owner drag_owner;

	തിരഞ്ഞെടുപ്പ്(scrollbar_data->സന്ദേശം) {
	അവസ്ഥ SCROLLBAR_സന്ദേശം_MOVED:

		എങ്കിൽ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			നിർത്തുക;
		}

		html__redraw_a_box(html, box);
		നിർത്തുക;
	അവസ്ഥ SCROLLBAR_സന്ദേശം_SCROLL_START:
	{
		ഘടന rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		നിർത്തുക;
	അവസ്ഥ SCROLLBAR_സന്ദേശം_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, ശൂന്യ);

		സന്ദേശം_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_സന്ദേശം_POINTER, &സന്ദേശം_data);
		നിർത്തുക;
	}
}


/* Documented in html_internal.h */
ശൂന്യം html_set_drag_type(html_content *html, html_drag_type drag_type,
		സംഘം html_drag_owner drag_owner, സ്ഥിരം ഘടന rect *rect)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;

	പ്രതിജ്ഞാ(html != ശൂന്യ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	തിരഞ്ഞെടുപ്പ് (drag_type) {
	അവസ്ഥ HTML_DRAG_NONE:
		പ്രതിജ്ഞാ(drag_owner.no_owner == true);
		സന്ദേശം_data.drag.type = CONTENT_DRAG_NONE;
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_SCROLLBAR:
	അവസ്ഥ HTML_DRAG_TEXTAREA_SCROLLBAR:
	അവസ്ഥ HTML_DRAG_CONTENT_SCROLL:
		സന്ദേശം_data.drag.type = CONTENT_DRAG_SCROLL;
		നിർത്തുക;

	അവസ്ഥ HTML_DRAG_SELECTION:
		പ്രതിജ്ഞാ(drag_owner.no_owner == true);
		fallthrough;
	അവസ്ഥ HTML_DRAG_TEXTAREA_SELECTION:
	അവസ്ഥ HTML_DRAG_CONTENT_SELECTION:
		സന്ദേശം_data.drag.type = CONTENT_DRAG_SELECTION;
		നിർത്തുക;
	}
	സന്ദേശം_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((ഘടന content *)html, CONTENT_സന്ദേശം_DRAG, &സന്ദേശം_data);
}

/* Documented in html_internal.h */
ശൂന്യം html_set_focus(html_content *html, html_focus_type focus_type,
		സംഘം html_focus_owner focus_owner, bool hide_caret,
		പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ height, സ്ഥിരം ഘടന rect *clip)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	പൂർണ്ണസംഖ്യ x_off = 0;
	പൂർണ്ണസംഖ്യ y_off = 0;
	ഘടന rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	പ്രതിജ്ഞാ(html != ശൂന്യ);

	തിരഞ്ഞെടുപ്പ് (focus_type) {
	അവസ്ഥ HTML_FOCUS_SELF:
		പ്രതിജ്ഞാ(focus_owner.self == true);
		എങ്കിൽ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			തിരികെ;
		നിർത്തുക;

	അവസ്ഥ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		നിർത്തുക;

	അവസ്ഥ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		നിർത്തുക;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	എങ്കിൽ (textarea_lost_focus) {
		സന്ദേശം_data.caret.type = CONTENT_CARET_REMOVE;
	} അല്ലെങ്കിൽ എങ്കിൽ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		സന്ദേശം_data.caret.type = CONTENT_CARET_HIDE;
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (clip != ശൂന്യ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		സന്ദേശം_data.caret.type = CONTENT_CARET_SET_POS;
		സന്ദേശം_data.caret.pos.x = x + x_off;
		സന്ദേശം_data.caret.pos.y = y + y_off;
		സന്ദേശം_data.caret.pos.height = height;
		സന്ദേശം_data.caret.pos.clip = (clip == ശൂന്യ) ? ശൂന്യ : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((ഘടന content *)html, CONTENT_സന്ദേശം_CARET, &സന്ദേശം_data);
}

/* Documented in html_internal.h */
ശൂന്യം html_set_selection(html_content *html, html_selection_type selection_type,
		സംഘം html_selection_owner selection_owner, bool read_only)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	ഘടന box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	പ്രതിജ്ഞാ(html != ശൂന്യ);

	എങ്കിൽ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	എങ്കിൽ (html->selection_type != HTML_SELECTION_NONE) {
		തിരഞ്ഞെടുപ്പ് (html->selection_type) {
		അവസ്ഥ HTML_SELECTION_SELF:
			എങ്കിൽ (same_type)
				നിർത്തുക;
			selection_clear(html->sel, true);
			നിർത്തുക;
		അവസ്ഥ HTML_SELECTION_TEXTAREA:
			എങ്കിൽ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				നിർത്തുക;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			നിർത്തുക;
		അവസ്ഥ HTML_SELECTION_CONTENT:
			എങ്കിൽ (same_type && html->selection_owner.content ==
					selection_owner.content)
				നിർത്തുക;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	എങ്കിൽ (!changed)
		/* Don't need to report lack of change to owner */
		തിരികെ;

	/* Prepare msg */
	തിരഞ്ഞെടുപ്പ് (selection_type) {
	അവസ്ഥ HTML_SELECTION_NONE:
		പ്രതിജ്ഞാ(selection_owner.none == true);
		സന്ദേശം_data.selection.selection = false;
		നിർത്തുക;
	അവസ്ഥ HTML_SELECTION_SELF:
		പ്രതിജ്ഞാ(selection_owner.none == false);
		fallthrough;
	അവസ്ഥ HTML_SELECTION_TEXTAREA:
	അവസ്ഥ HTML_SELECTION_CONTENT:
		സന്ദേശം_data.selection.selection = true;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}
	സന്ദേശം_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((ഘടന content *)html, CONTENT_സന്ദേശം_SELECTION,
			&സന്ദേശം_data);
}
