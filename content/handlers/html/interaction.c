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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ <dom/dom.h>

#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "netsurf/browser_window.h"
#অন্তর্ভাৱ "netsurf/mouse.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "netsurf/layout.h"
#অন্তর্ভাৱ "netsurf/keypress.h"
#অন্তর্ভাৱ "content/hlcache.h"
#অন্তর্ভাৱ "content/textsearch.h"
#অন্তর্ভাৱ "desktop/browser_history.h"
#অন্তর্ভাৱ "desktop/frames.h"
#অন্তর্ভাৱ "desktop/scrollbar.h"
#অন্তর্ভাৱ "desktop/selection.h"
#অন্তর্ভাৱ "desktop/textarea.h"
#অন্তর্ভাৱ "javascript/js.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/box_textarea.h"
#অন্তর্ভাৱ "html/box_inspect.h"
#অন্তর্ভাৱ "html/font.h"
#অন্তর্ভাৱ "html/form_internal.h"
#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/imagemap.h"
#অন্তর্ভাৱ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

স্থির browser_pointer_shape get_pointer_shape(সংরচনা box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	গণনা css_cursor_e cursor;
	lwc_string **cursor_uris;

	যদি (box->type == BOX_FLOAT_বাম || box->type == BOX_FLOAT_ডান)
		style = box->children->style;
	অন্যথা
		style = box->style;

	যদি (style == রিক্ত)
		ঘূৰাই BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ৱিকল্প (cursor) {
	পক্ষ CSS_CURSOR_AUTO:
		যদি (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} অন্যথা যদি (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} অন্যথা {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		ভঙ্গ;
	পক্ষ CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_ডান;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_বাম;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		ভঙ্গ;
	পক্ষ CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		ভঙ্গ;
	স্ৱভাৱ:
		pointer = BROWSER_POINTER_DEFAULT;
		ভঙ্গ;
	}

	ঘূৰাই pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

স্থির শূন্য html_box_drag_start(সংরচনা box *box, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	পূর্ণাঙ্ক box_x, box_y;
	পূর্ণাঙ্ক scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	যদি (box->scroll_x != রিক্ত) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[শীৰ্ষ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} অন্যথা যদি (box->scroll_y != রিক্ত) {
		scroll_mouse_x = x - (box_x + box->padding[বাম_2] +
				box->width + box->padding[ডান_2] -
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
স্থির size_t html_selection_drag_end(সংরচনা html_content *html,
		browser_mouse_state mouse, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক dir)
{
	পূর্ণাঙ্ক pixel_offset;
	সংরচনা box *box;
	পূর্ণাঙ্ক dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	যদি (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	ঘূৰাই idx;
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
স্থির শূন্য
html__image_coords_dom_user_data_handler(dom_গ্ৰন্থি_operation operation,
					 dom_string *চাবি,
					 শূন্য *_data,
					 সংরচনা dom_গ্ৰন্থি *src,
					 সংরচনা dom_গ্ৰন্থি *dst)
{
	সংরচনা image_প্ৰবেশ_coords *oldcoords, *coords = _data, *newcoords;

	যদি (!dom_string_isequal(corestring_dom_ns_চাবি_image_coords_গ্ৰন্থি_data,
				চাবি) || coords == রিক্ত) {
		ঘূৰাই;
	}

	ৱিকল্প (operation) {
	পক্ষ DOM_গ্ৰন্থি_CLONED:
		newcoords = calloc(1, পরিমাণ(*newcoords));
		যদি (newcoords != রিক্ত) {
			*newcoords = *coords;
			যদি (dom_গ্ৰন্থি_set_user_data(dst,
				 corestring_dom_ns_চাবি_image_coords_গ্ৰন্থি_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				মুক্ত(oldcoords);
			}
		}
		ভঙ্গ;

	পক্ষ DOM_গ্ৰন্থি_DELETED:
		মুক্ত(coords);
		ভঙ্গ;

	পক্ষ DOM_গ্ৰন্থি_RENAMED:
	পক্ষ DOM_গ্ৰন্থি_IMPORTED:
	পক্ষ DOM_গ্ৰন্থি_ADOPTED:
		ভঙ্গ;

	স্ৱভাৱ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		প্রতিজ্ঞা(0);
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
স্থির শূন্য
html_overflow_scroll_drag_end(সংরচনা scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	পূর্ণাঙ্ক scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	সংরচনা html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	সংরচনা box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	যদি (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[শীৰ্ষ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} অন্যথা {
		scroll_mouse_x = x - (box_x + box->padding[বাম_2] +
				box->width + box->padding[ডান_2] -
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
স্থির nserror
mouse_action_select_menu(html_content *html,
			 সংরচনা browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা box *box;
	পূর্ণাঙ্ক box_x = 0;
	পূর্ণাঙ্ক box_y = 0;
	ধ্রুৱ ৱর্ণ *status;
	পূর্ণাঙ্ক width, height;
	সংরচনা hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	প্রতিজ্ঞা(html->visible_select_menu != রিক্ত);

	bw_drag_type = browser_window_get_drag_type(bw);
	যদি (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[বাম_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[শীৰ্ষ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	যদি (status != রিক্ত) {
		/* set status if menu still open */
		সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
		বাৰ্তা_data.explicit_status_text = status;
		content_broadcast((সংরচনা content *)html,
				  CONTENT_বাৰ্তা_STATUS,
				  &বাৰ্তা_data);
		ঘূৰাই NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = রিক্ত;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	ঘূৰাই NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
স্থির nserror
mouse_action_drag_selection(html_content *html,
			    সংরচনা browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা box *box;
	পূর্ণাঙ্ক dir = -1;
	পূর্ণাঙ্ক dx, dy;
	size_t idx;
	সঙ্ঘ html_drag_owner drag_owner;
	পূর্ণাঙ্ক pixel_offset;
	plot_font_style_t fstyle;

	যদি (!mouse) {
		/* End of selection drag */
		যদি (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		যদি (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, রিক্ত);

		ঘূৰাই NSERROR_OK;
	}

	যদি (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	যদি (box != রিক্ত) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	ঘূৰাই NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
স্থির nserror
mouse_action_drag_scrollbar(html_content *html,
			    সংরচনা browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা scrollbar *scr;
	সংরচনা html_scrollbar_data *data;
	সংরচনা box *box;
	পূর্ণাঙ্ক box_x = 0;
	পূর্ণাঙ্ক box_y = 0;
	ধ্রুৱ ৱর্ণ *status;
	পূর্ণাঙ্ক scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	যদি (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	যদি (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[শীৰ্ষ_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} অন্যথা {
		scroll_mouse_x = x - (box_x + box->padding[বাম_2] +
				      box->width + box->padding[ডান_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	যদি (status != রিক্ত) {
		সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

		বাৰ্তা_data.explicit_status_text = status;
		content_broadcast((সংরচনা content *)html,
				  CONTENT_বাৰ্তা_STATUS,
				  &বাৰ্তা_data);
	}

	ঘূৰাই NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
স্থির nserror
mouse_action_drag_textarea(html_content *html,
			    সংরচনা browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা box *box;
	পূর্ণাঙ্ক box_x = 0;
	পূর্ণাঙ্ক box_y = 0;

	box = html->drag_owner.textarea;

	প্রতিজ্ঞা(box->gadget != রিক্ত);
	প্রতিজ্ঞা(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ঘূৰাই NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
স্থির nserror
mouse_action_drag_content(html_content *html,
			  সংরচনা browser_window *bw,
			  browser_mouse_state mouse,
			  পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা box *box;
	পূর্ণাঙ্ক box_x = 0;
	পূর্ণাঙ্ক box_y = 0;

	box = html->drag_owner.content;
	প্রতিজ্ঞা(box->object != রিক্ত);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ঘূৰাই NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
সংরচনা mouse_action_state {
	সংরচনা {
		ধ্রুৱ ৱর্ণ *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		গণনা {
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
	সংরচনা dom_গ্ৰন্থি *গ্ৰন্থি;

	/** html object */
	সংরচনা {
		সংরচনা box *box;
		পূর্ণাঙ্ক pos_x;
		পূর্ণাঙ্ক pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	সংরচনা browser_window *iframe;

	/** link either from href or imagemap */
	সংরচনা {
		সংরচনা box *box;
		nsurl *url;
		ধ্রুৱ ৱর্ণ *target;
		bool is_imagemap;
	} link;

	/** gadget */
	সংরচনা {
		সংরচনা form_control *control;
		সংরচনা box *box;
		পূর্ণাঙ্ক box_x;
		পূর্ণাঙ্ক box_y;
		ধ্রুৱ ৱর্ণ *target;
	} gadget;

	/** title */
	ধ্রুৱ ৱর্ণ *title;

	/** candidate box for drag operation */
	সংরচনা box *drag_candidate;

	/** scrollbar */
	সংরচনা {
		সংরচনা scrollbar *bar;
		পূর্ণাঙ্ক mouse_x;
		পূর্ণাঙ্ক mouse_y;
	} scroll;

	/** text in box */
	সংরচনা {
		সংরচনা box *box;
		পূর্ণাঙ্ক box_x;
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
স্থির nserror
get_mouse_action_গ্ৰন্থি(html_content *html,
		      পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		      সংরচনা mouse_action_state *man)
{
	সংরচনা box *box;
	পূর্ণাঙ্ক box_x = 0;
	পূর্ণাঙ্ক box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, পরিমাণ(সংরচনা mouse_action_state));
	man->গ্ৰন্থি = html->layout->গ্ৰন্থি; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[বাম_2];
	box_y = box->margin[শীৰ্ষ_2];

	কৰা {
		/* skip hidden boxes */
		যদি ((box->style != রিক্ত) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			গচ্ছ next_box;
		}

		যদি (box->গ্ৰন্থি != রিক্ত) {
			man->গ্ৰন্থি = box->গ্ৰন্থি;
		}

		যদি (box->object) {
			যদি (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} অন্যথা {
				man->object = box->object;
			}
		}

		যদি (box->iframe) {
			man->iframe = box->iframe;
		}

		যদি (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		যদি (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		যদি (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			যদি (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		যদি (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		যদি ((box->scroll_x != রিক্ত) ||
		    (box->scroll_y != রিক্ত)) {
			পূর্ণাঙ্ক padding_বাম;
			পূর্ণাঙ্ক padding_ডান;
			পূর্ণাঙ্ক padding_শীৰ্ষ;
			পূর্ণাঙ্ক padding_bottom;

			যদি (man->drag_candidate == রিক্ত) {
				man->drag_candidate = box;
			}

			padding_বাম = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_ডান = padding_বাম + box->padding[বাম_2] +
					box->width + box->padding[ডান_2];
			padding_শীৰ্ষ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_শীৰ্ষ + box->padding[শীৰ্ষ_2] +
					box->height + box->padding[BOTTOM];

			যদি ((x > padding_বাম) &&
			    (x < padding_ডান) &&
			    (y > padding_শীৰ্ষ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				যদি ((box->scroll_y != রিক্ত) &&
				    (x > (padding_ডান - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ডান - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_শীৰ্ষ;
					ভঙ্গ;

				} অন্যথা যদি ((box->scroll_x != রিক্ত) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_বাম;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					ভঙ্গ;
				}
			}
		}

		যদি (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} যেতিয়ালৈ (box != রিক্ত);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	প্রতিজ্ঞা(man->গ্ৰন্থি != রিক্ত);

	ঘূৰাই NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
স্থির nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		    সংরচনা mouse_action_state *mas)
{
	সংরচনা content *c = (সংরচনা content *)html;
	textarea_mouse_status ta_status;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ৱিকল্প (mas->gadget.control->type) {
	পক্ষ GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		যদি (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			যদি (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = রিক্ত;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			বাৰ্তা_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_বাৰ্তা_SELECTMENU,
					  &বাৰ্তা_data);
		}
		ভঙ্গ;

	পক্ষ GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_প্ৰবেশ_element_set_checked(
				(dom_html_প্ৰবেশ_element *)(mas->gadget.control->গ্ৰন্থি),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ভঙ্গ;

	পক্ষ GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		ভঙ্গ;

	পক্ষ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			সংরচনা image_প্ৰবেশ_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, পরিমাণ(*coords));
			যদি (coords == রিক্ত) {
				ঘূৰাই NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			যদি (dom_গ্ৰন্থি_set_user_data(
				mas->gadget.control->গ্ৰন্থি,
				corestring_dom_ns_চাবি_image_coords_গ্ৰন্থি_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				ঘূৰাই NSERROR_OK;
			}
			মুক্ত(oldcoords);
		}
		fallthrough;

	পক্ষ GADGET_SUBMIT:
		যদি (mas->gadget.control->form) {
			স্থির ৱর্ণ status_buffer[200];

			snprintf(status_buffer,
				 পরিমাণ status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			যদি (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} অন্যথা {
			mas->result.status = messages_get("FormBadSubmit");
		}
		ভঙ্গ;

	পক্ষ GADGET_TEXTBOX:
	পক্ষ GADGET_PASSWORD:
	পক্ষ GADGET_TEXTAREA:
		যদি (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} অন্যথা {
			mas->result.status = messages_get("FormTextbox");
		}

		যদি (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			সঙ্ঘ html_selection_owner sel_owner;
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

		যদি (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} অন্যথা {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		ভঙ্গ;

	পক্ষ GADGET_HIDDEN:
		/* not possible: no box generated */
		ভঙ্গ;

	পক্ষ GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		ভঙ্গ;

	পক্ষ GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			বাৰ্তা_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_বাৰ্তা_GADGETCLICK,
					  &বাৰ্তা_data);
		}
		ভঙ্গ;

	পক্ষ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		ভঙ্গ;
	}

	ঘূৰাই NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
স্থির nserror
iframe_mouse_action(সংরচনা browser_window *bw,
		    browser_mouse_state mouse,
		    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		    সংরচনা mouse_action_state *mas)
{
	পূর্ণাঙ্ক pos_x, pos_y;
	ভিন্ন scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	যদি (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} অন্যথা {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	ঘূৰাই NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
স্থির nserror
html_object_mouse_action(html_content *html,
			 সংরচনা browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
			 সংরচনা mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	যদি (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		সঙ্ঘ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	যদি (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} অন্যথা {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	ঘূৰাই NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
স্থির bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	যদি (scheme != রিক্ত) {
		যদি (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ঘূৰাই is_js;
}


/**
 * process mouse activity on a link
 */
স্থির nserror
link_mouse_action(html_content *html,
		  সংরচনা browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		  সংরচনা mouse_action_state *mas)
{
	nserror res;
	ৱর্ণ *url_s = রিক্ত;
	size_t url_l = 0;
	স্থির ৱর্ণ status_buffer[200];
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

	যদি (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		যদি (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = রিক্ত;
		}
	}

	যদি (mas->title) {
		snprintf(status_buffer,
			 পরিমাণ status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} অন্যথা {
		snprintf(status_buffer,
			 পরিমাণ status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	যদি (url_s != রিক্ত) {
		মুক্ত(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	যদি (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((সংরচনা content *)html),
					BW_NAVIGATE_DOWNLOAD,
					রিক্ত,
					রিক্ত,
					রিক্ত);

	} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		বাৰ্তা_data.savelink.url = mas->link.url;
		বাৰ্তা_data.savelink.title = mas->title;
		content_broadcast((সংরচনা content *)html,
				  CONTENT_বাৰ্তা_SAVELINK,
				  &বাৰ্তা_data);

	} অন্যথা যদি (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		যদি (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} অন্যথা {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	ঘূৰাই NSERROR_OK;
}


স্থির nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	যদি (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		সঙ্ঘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, রিক্ত);
	}

	ঘূৰাই NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
স্থির nserror
default_mouse_action(html_content *html,
		  সংরচনা browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		  সংরচনা mouse_action_state *mas)
{
	সংরচনা content *c = (সংরচনা content *)html;

	/* frame resizing */
	যদি (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		যদি (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		ঘূৰাই default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	সঙ্ঘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	যদি (click && html->focus_type != HTML_FOCUS_SELF) {
		সঙ্ঘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, রিক্ত);
	}
	যদি (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	যদি (mas->text.box) {
		পূর্ণাঙ্ক pixel_offset;
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

		যদি (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			সঙ্ঘ html_drag_owner drag_owner;

			যদি (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   রিক্ত);
				mas->result.status = messages_get("Selecting");
			}

			যদি (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} অন্যথা যদি (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ঘূৰাই default_mouse_action_focus(html, mouse);
		}

	} অন্যথা যদি (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	যদি (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} অন্যথা যদি (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	যদি (mas->title) {
		mas->result.status = mas->title;
	}

	যদি (mouse & BROWSER_MOUSE_DRAG_1) {
		যদি (mouse & BROWSER_MOUSE_MOD_2) {
			সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
			বাৰ্তা_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			বাৰ্তা_data.dragsave.content = রিক্ত;
			content_broadcast(c, CONTENT_বাৰ্তা_DRAGSAVE, &বাৰ্তা_data);
		} অন্যথা {
			যদি (mas->drag_candidate == রিক্ত) {
				browser_window_page_drag_start(bw, x, y);
			} অন্যথা {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} অন্যথা যদি (mouse & BROWSER_MOUSE_DRAG_2) {
		যদি (mouse & BROWSER_MOUSE_MOD_2) {
			সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
			বাৰ্তা_data.dragsave.type = CONTENT_SAVE_SOURCE;
			বাৰ্তা_data.dragsave.content = রিক্ত;
			content_broadcast(c, CONTENT_বাৰ্তা_DRAGSAVE, &বাৰ্তা_data);
		} অন্যথা {
			যদি (mas->drag_candidate == রিক্ত) {
				browser_window_page_drag_start(bw, x, y);
			} অন্যথা {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	ঘূৰাই default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
স্থির nserror
mouse_action_drag_none(html_content *html,
		       সংরচনা browser_window *bw,
		       browser_mouse_state mouse,
		       পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	nserror res;
	সংরচনা content *c = (সংরচনা content *)html;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	স্থির সংরচনা mouse_action_state mas;

	res = get_mouse_action_গ্ৰন্থি(html, x, y, &mas);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	যদি (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} অন্যথা যদি (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} অন্যথা যদি (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} অন্যথা যদি ((mas.object != রিক্ত) && (mouse & BROWSER_MOUSE_MOD_2)) {

		যদি (mouse & BROWSER_MOUSE_DRAG_2) {
			বাৰ্তা_data.dragsave.type = CONTENT_SAVE_NATIVE;
			বাৰ্তা_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_বাৰ্তা_DRAGSAVE, &বাৰ্তা_data);

		} অন্যথা যদি (mouse & BROWSER_MOUSE_DRAG_1) {
			বাৰ্তা_data.dragsave.type = CONTENT_SAVE_ORIG;
			বাৰ্তা_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_বাৰ্তা_DRAGSAVE, &বাৰ্তা_data);
		}

		/* \todo should have a drag-saving object msg */

	} অন্যথা যদি (mas.iframe != রিক্ত) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} অন্যথা যদি (mas.html_object.box != রিক্ত) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} অন্যথা যদি (mas.link.url != রিক্ত) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} অন্যথা {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	/* send status and pointer message */
	যদি (mas.result.action != ACTION_NOSEND) {
		বাৰ্তা_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_বাৰ্তা_STATUS, &বাৰ্তা_data);

		বাৰ্তা_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_বাৰ্তা_POINTER, &বাৰ্তা_data);
	}

	/* fire dom click event */
	যদি (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.গ্ৰন্থি, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ৱিকল্প (mas.result.action) {
	পক্ষ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ভঙ্গ;

	পক্ষ ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				রিক্ত,
				রিক্ত,
				রিক্ত);
		ভঙ্গ;

	পক্ষ ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		যদি (path != রিক্ত) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		ভঙ্গ;

	পক্ষ ACTION_BACK:
		res = browser_window_history_back(bw, false);
		ভঙ্গ;

	পক্ষ ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		ভঙ্গ;

	পক্ষ ACTION_NOSEND:
	পক্ষ ACTION_NONE:
		res = NSERROR_OK;
		ভঙ্গ;
	}

	ঘূৰাই res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(সংরচনা content *c,
			 সংরচনা browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	ঘূৰাই html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(সংরচনা content *c,
		  সংরচনা browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	যদি (html->visible_select_menu != রিক্ত) {
		ঘূৰাই mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ৱিকল্প (html->drag_type) {
	পক্ষ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ভঙ্গ;

	পক্ষ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ভঙ্গ;

	পক্ষ HTML_DRAG_TEXTAREA_SELECTION:
	পক্ষ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ভঙ্গ;

	পক্ষ HTML_DRAG_CONTENT_SELECTION:
	পক্ষ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ভঙ্গ;

	পক্ষ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ভঙ্গ;

	স্ৱভাৱ:
		/* Unknown content related drag type */
		প্রতিজ্ঞা(0 && "Unknown content related drag type");
	}

	যদি (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ঘূৰাই res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(সংরচনা content *c, uint32_t চাবি)
{
	html_content *html = (html_content *) c;
	সংরচনা selection *sel = html->sel;

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
	যদি (html->layout != রিক্ত && html->layout->গ্ৰন্থি != রিক্ত) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->গ্ৰন্থি, true, true, চাবি);
	}

	ৱিকল্প (html->focus_type) {
	পক্ষ HTML_FOCUS_CONTENT:
		ঘূৰাই content_keypress(html->focus_owner.content->object, চাবি);

	পক্ষ HTML_FOCUS_TEXTAREA:
		যদি (box_textarea_keypress(html, html->focus_owner.textarea, চাবি) == NSERROR_OK) {
			ঘূৰাই true;
		} অন্যথা {
			ঘূৰাই false;
		}

	স্ৱভাৱ:
		/* Deal with it below */
		ভঙ্গ;
	}

	ৱিকল্প (চাবি) {
	পক্ষ NS_চাবি_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ঘূৰাই true;

	পক্ষ NS_চাবি_CLEAR_SELECTION:
		selection_clear(sel, true);
		ঘূৰাই true;

	পক্ষ NS_চাবি_SELECT_ALL:
		selection_select_all(sel);
		ঘূৰাই true;

	পক্ষ NS_চাবি_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ঘূৰাই selection_clear(sel, true);
	}

	ঘূৰাই false;
}


/**
 * Callback for in-page scrollbars.
 */
শূন্য html_overflow_scroll_callback(শূন্য *client_data,
		সংরচনা scrollbar_বাৰ্তা_data *scrollbar_data)
{
	সংরচনা html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	সংরচনা box *box = data->box;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	html_drag_type drag_type;
	সঙ্ঘ html_drag_owner drag_owner;

	ৱিকল্প(scrollbar_data->বাৰ্তা) {
	পক্ষ SCROLLBAR_বাৰ্তা_MOVED:

		যদি (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ভঙ্গ;
		}

		html__redraw_a_box(html, box);
		ভঙ্গ;
	পক্ষ SCROLLBAR_বাৰ্তা_SCROLL_START:
	{
		সংরচনা rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		ভঙ্গ;
	পক্ষ SCROLLBAR_বাৰ্তা_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, রিক্ত);

		বাৰ্তা_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_বাৰ্তা_POINTER, &বাৰ্তা_data);
		ভঙ্গ;
	}
}


/* Documented in html_internal.h */
শূন্য html_set_drag_type(html_content *html, html_drag_type drag_type,
		সঙ্ঘ html_drag_owner drag_owner, ধ্রুৱ সংরচনা rect *rect)
{
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

	প্রতিজ্ঞা(html != রিক্ত);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ৱিকল্প (drag_type) {
	পক্ষ HTML_DRAG_NONE:
		প্রতিজ্ঞা(drag_owner.no_owner == true);
		বাৰ্তা_data.drag.type = CONTENT_DRAG_NONE;
		ভঙ্গ;

	পক্ষ HTML_DRAG_SCROLLBAR:
	পক্ষ HTML_DRAG_TEXTAREA_SCROLLBAR:
	পক্ষ HTML_DRAG_CONTENT_SCROLL:
		বাৰ্তা_data.drag.type = CONTENT_DRAG_SCROLL;
		ভঙ্গ;

	পক্ষ HTML_DRAG_SELECTION:
		প্রতিজ্ঞা(drag_owner.no_owner == true);
		fallthrough;
	পক্ষ HTML_DRAG_TEXTAREA_SELECTION:
	পক্ষ HTML_DRAG_CONTENT_SELECTION:
		বাৰ্তা_data.drag.type = CONTENT_DRAG_SELECTION;
		ভঙ্গ;
	}
	বাৰ্তা_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((সংরচনা content *)html, CONTENT_বাৰ্তা_DRAG, &বাৰ্তা_data);
}

/* Documented in html_internal.h */
শূন্য html_set_focus(html_content *html, html_focus_type focus_type,
		সঙ্ঘ html_focus_owner focus_owner, bool hide_caret,
		পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক height, ধ্রুৱ সংরচনা rect *clip)
{
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	পূর্ণাঙ্ক x_off = 0;
	পূর্ণাঙ্ক y_off = 0;
	সংরচনা rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	প্রতিজ্ঞা(html != রিক্ত);

	ৱিকল্প (focus_type) {
	পক্ষ HTML_FOCUS_SELF:
		প্রতিজ্ঞা(focus_owner.self == true);
		যদি (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ঘূৰাই;
		ভঙ্গ;

	পক্ষ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ভঙ্গ;

	পক্ষ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ভঙ্গ;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	যদি (textarea_lost_focus) {
		বাৰ্তা_data.caret.type = CONTENT_CARET_REMOVE;
	} অন্যথা যদি (focus_type != HTML_FOCUS_SELF && hide_caret) {
		বাৰ্তা_data.caret.type = CONTENT_CARET_HIDE;
	} অন্যথা {
		যদি (clip != রিক্ত) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		বাৰ্তা_data.caret.type = CONTENT_CARET_SET_POS;
		বাৰ্তা_data.caret.pos.x = x + x_off;
		বাৰ্তা_data.caret.pos.y = y + y_off;
		বাৰ্তা_data.caret.pos.height = height;
		বাৰ্তা_data.caret.pos.clip = (clip == রিক্ত) ? রিক্ত : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((সংরচনা content *)html, CONTENT_বাৰ্তা_CARET, &বাৰ্তা_data);
}

/* Documented in html_internal.h */
শূন্য html_set_selection(html_content *html, html_selection_type selection_type,
		সঙ্ঘ html_selection_owner selection_owner, bool read_only)
{
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	সংরচনা box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	প্রতিজ্ঞা(html != রিক্ত);

	যদি ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	যদি (html->selection_type != HTML_SELECTION_NONE) {
		ৱিকল্প (html->selection_type) {
		পক্ষ HTML_SELECTION_SELF:
			যদি (same_type)
				ভঙ্গ;
			selection_clear(html->sel, true);
			ভঙ্গ;
		পক্ষ HTML_SELECTION_TEXTAREA:
			যদি (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ভঙ্গ;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			ভঙ্গ;
		পক্ষ HTML_SELECTION_CONTENT:
			যদি (same_type && html->selection_owner.content ==
					selection_owner.content)
				ভঙ্গ;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ভঙ্গ;
		স্ৱভাৱ:
			ভঙ্গ;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	যদি (!changed)
		/* Don't need to report lack of change to owner */
		ঘূৰাই;

	/* Prepare msg */
	ৱিকল্প (selection_type) {
	পক্ষ HTML_SELECTION_NONE:
		প্রতিজ্ঞা(selection_owner.none == true);
		বাৰ্তা_data.selection.selection = false;
		ভঙ্গ;
	পক্ষ HTML_SELECTION_SELF:
		প্রতিজ্ঞা(selection_owner.none == false);
		fallthrough;
	পক্ষ HTML_SELECTION_TEXTAREA:
	পক্ষ HTML_SELECTION_CONTENT:
		বাৰ্তা_data.selection.selection = true;
		ভঙ্গ;
	স্ৱভাৱ:
		ভঙ্গ;
	}
	বাৰ্তা_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((সংরচনা content *)html, CONTENT_বাৰ্তা_SELECTION,
			&বাৰ্তা_data);
}
