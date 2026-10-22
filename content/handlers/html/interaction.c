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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/mouse.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "netsurf/layout.h"
#অন্তর্ভাব "netsurf/keypress.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "content/textsearch.h"
#অন্তর্ভাব "desktop/browser_history.h"
#অন্তর্ভাব "desktop/frames.h"
#অন্তর্ভাব "desktop/scrollbar.h"
#অন্তর্ভাব "desktop/selection.h"
#অন্তর্ভাব "desktop/textarea.h"
#অন্তর্ভাব "javascript/js.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_textarea.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/form_internal.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/imagemap.h"
#অন্তর্ভাব "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

স্থির browser_pointer_shape get_pointer_shape(কাঠামো box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	গণনা css_cursor_e cursor;
	lwc_string **cursor_uris;

	যদি (box->type == BOX_FLOAT_বাম || box->type == BOX_FLOAT_ডান)
		style = box->children->style;
	অন্যথা
		style = box->style;

	যদি (style == ফাঁকা)
		ফেরত BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	নির্বাচন (cursor) {
	ক্ষেত্র CSS_CURSOR_AUTO:
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
		বিরতি;
	ক্ষেত্র CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_ডান;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_বাম;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		বিরতি;
	ক্ষেত্র CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		বিরতি;
	পূর্বনির্ধারিত:
		pointer = BROWSER_POINTER_DEFAULT;
		বিরতি;
	}

	ফেরত pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

স্থির শূন্য html_box_drag_start(কাঠামো box *box, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	পূর্ণসংখ্যা box_x, box_y;
	পূর্ণসংখ্যা scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	যদি (box->scroll_x != ফাঁকা) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[শীর্ষ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} অন্যথা যদি (box->scroll_y != ফাঁকা) {
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
স্থির size_t html_selection_drag_end(কাঠামো html_content *html,
		browser_mouse_state mouse, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা dir)
{
	পূর্ণসংখ্যা pixel_offset;
	কাঠামো box *box;
	পূর্ণসংখ্যা dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	যদি (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->দৈর্ঘ্য,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	ফেরত idx;
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
html_image_coords_dom_user_তথ্য_handler(dom_গ্রন্থি_operation operation,
					 dom_string *চাবি,
					 শূন্য *তথ্য_2,
					 কাঠামো dom_গ্রন্থি *src,
					 কাঠামো dom_গ্রন্থি *dst)
{
	কাঠামো image_প্রবেশ_coords *oldcoords, *coords = তথ্য_2, *newcoords;

	যদি (!dom_string_isequal(corestring_dom_ns_চাবি_image_coords_গ্রন্থি_তথ্য,
				চাবি) || coords == ফাঁকা) {
		ফেরত;
	}

	নির্বাচন (operation) {
	ক্ষেত্র DOM_গ্রন্থি_CLONED:
		newcoords = calloc(1, আকার(*newcoords));
		যদি (newcoords != ফাঁকা) {
			*newcoords = *coords;
			যদি (dom_গ্রন্থি_set_user_তথ্য(dst,
				 corestring_dom_ns_চাবি_image_coords_গ্রন্থি_তথ্য,
				 newcoords,
				 html_image_coords_dom_user_তথ্য_handler,
				 &oldcoords) == DOM_NO_ERR) {
				মুক্ত(oldcoords);
			}
		}
		বিরতি;

	ক্ষেত্র DOM_গ্রন্থি_DELETED:
		মুক্ত(coords);
		বিরতি;

	ক্ষেত্র DOM_গ্রন্থি_RENAMED:
	ক্ষেত্র DOM_গ্রন্থি_IMPORTED:
	ক্ষেত্র DOM_গ্রন্থি_ADOPTED:
		বিরতি;

	পূর্বনির্ধারিত:
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
html_overflow_scroll_drag_end(কাঠামো scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	পূর্ণসংখ্যা scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	কাঠামো html_scrollbar_তথ্য *তথ্য = scrollbar_get_তথ্য(scrollbar);
	কাঠামো box *box;

	box = তথ্য->box;
	box_coords(box, &box_x, &box_y);

	যদি (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[শীর্ষ_2] +
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
			 কাঠামো browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা box_x = 0;
	পূর্ণসংখ্যা box_y = 0;
	ধ্রুবক অক্ষর *status;
	পূর্ণসংখ্যা width, height;
	কাঠামো hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	প্রতিজ্ঞা(html->visible_select_menu != ফাঁকা);

	bw_drag_type = browser_window_get_drag_type(bw);
	যদি (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[বাম_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[শীর্ষ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	যদি (status != ফাঁকা) {
		/* set status if menu still open */
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
		বার্তা_তথ্য.explicit_status_text = status;
		content_broadcast((কাঠামো content *)html,
				  CONTENT_বার্তা_STATUS,
				  &বার্তা_তথ্য);
		ফেরত NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = ফাঁকা;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	ফেরত NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
স্থির nserror
mouse_action_drag_selection(html_content *html,
			    কাঠামো browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা dir = -1;
	পূর্ণসংখ্যা dx, dy;
	size_t idx;
	সংঘ html_drag_owner drag_owner;
	পূর্ণসংখ্যা pixel_offset;
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
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, ফাঁকা);

		ফেরত NSERROR_OK;
	}

	যদি (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	যদি (box != ফাঁকা) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->দৈর্ঘ্য,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	ফেরত NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
স্থির nserror
mouse_action_drag_scrollbar(html_content *html,
			    কাঠামো browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো scrollbar *scr;
	কাঠামো html_scrollbar_তথ্য *তথ্য;
	কাঠামো box *box;
	পূর্ণসংখ্যা box_x = 0;
	পূর্ণসংখ্যা box_y = 0;
	ধ্রুবক অক্ষর *status;
	পূর্ণসংখ্যা scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	যদি (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	তথ্য = scrollbar_get_তথ্য(scr);

	box = তথ্য->box;

	box_coords(box, &box_x, &box_y);

	যদি (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[শীর্ষ_2] +
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

	যদি (status != ফাঁকা) {
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

		বার্তা_তথ্য.explicit_status_text = status;
		content_broadcast((কাঠামো content *)html,
				  CONTENT_বার্তা_STATUS,
				  &বার্তা_তথ্য);
	}

	ফেরত NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
স্থির nserror
mouse_action_drag_textarea(html_content *html,
			    কাঠামো browser_window *bw,
			    browser_mouse_state mouse,
			    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা box_x = 0;
	পূর্ণসংখ্যা box_y = 0;

	box = html->drag_owner.textarea;

	প্রতিজ্ঞা(box->gadget != ফাঁকা);
	প্রতিজ্ঞা(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->তথ্য.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ফেরত NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
স্থির nserror
mouse_action_drag_content(html_content *html,
			  কাঠামো browser_window *bw,
			  browser_mouse_state mouse,
			  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা box_x = 0;
	পূর্ণসংখ্যা box_y = 0;

	box = html->drag_owner.content;
	প্রতিজ্ঞা(box->object != ফাঁকা);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ফেরত NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
কাঠামো mouse_action_state {
	কাঠামো {
		ধ্রুবক অক্ষর *status; /**< status text */
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
	} ফলাফল;

	/** dom node */
	কাঠামো dom_গ্রন্থি *গ্রন্থি;

	/** html object */
	কাঠামো {
		কাঠামো box *box;
		পূর্ণসংখ্যা pos_x;
		পূর্ণসংখ্যা pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	কাঠামো browser_window *iframe;

	/** link either from href or imagemap */
	কাঠামো {
		কাঠামো box *box;
		nsurl *url;
		ধ্রুবক অক্ষর *target;
		bool is_imagemap;
	} link;

	/** gadget */
	কাঠামো {
		কাঠামো form_control *control;
		কাঠামো box *box;
		পূর্ণসংখ্যা box_x;
		পূর্ণসংখ্যা box_y;
		ধ্রুবক অক্ষর *target;
	} gadget;

	/** title */
	ধ্রুবক অক্ষর *title;

	/** candidate box for drag operation */
	কাঠামো box *drag_candidate;

	/** scrollbar */
	কাঠামো {
		কাঠামো scrollbar *bar;
		পূর্ণসংখ্যা mouse_x;
		পূর্ণসংখ্যা mouse_y;
	} scroll;

	/** text in box */
	কাঠামো {
		কাঠামো box *box;
		পূর্ণসংখ্যা box_x;
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
get_mouse_action_গ্রন্থি(html_content *html,
		      পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		      কাঠামো mouse_action_state *man)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা box_x = 0;
	পূর্ণসংখ্যা box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, আকার(কাঠামো mouse_action_state));
	man->গ্রন্থি = html->layout->গ্রন্থি; /* Default dom node to the <HTML> */
	man->ফলাফল.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[বাম_2];
	box_y = box->margin[শীর্ষ_2];

	করো {
		/* skip hidden boxes */
		যদি ((box->style != ফাঁকা) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			যাও পরবর্তী_box;
		}

		যদি (box->গ্রন্থি != ফাঁকা) {
			man->গ্রন্থি = box->গ্রন্থি;
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

		man->ফলাফল.pointer = get_pointer_shape(box, false);

		যদি ((box->scroll_x != ফাঁকা) ||
		    (box->scroll_y != ফাঁকা)) {
			পূর্ণসংখ্যা padding_বাম;
			পূর্ণসংখ্যা padding_ডান;
			পূর্ণসংখ্যা padding_শীর্ষ;
			পূর্ণসংখ্যা padding_bottom;

			যদি (man->drag_candidate == ফাঁকা) {
				man->drag_candidate = box;
			}

			padding_বাম = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_ডান = padding_বাম + box->padding[বাম_2] +
					box->width + box->padding[ডান_2];
			padding_শীর্ষ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_শীর্ষ + box->padding[শীর্ষ_2] +
					box->height + box->padding[BOTTOM];

			যদি ((x > padding_বাম) &&
			    (x < padding_ডান) &&
			    (y > padding_শীর্ষ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				যদি ((box->scroll_y != ফাঁকা) &&
				    (x > (padding_ডান - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ডান - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_শীর্ষ;
					বিরতি;

				} অন্যথা যদি ((box->scroll_x != ফাঁকা) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_বাম;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					বিরতি;
				}
			}
		}

		যদি (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	পরবর্তী_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} যখন (box != ফাঁকা);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	প্রতিজ্ঞা(man->গ্রন্থি != ফাঁকা);

	ফেরত NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
স্থির nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		    কাঠামো mouse_action_state *mas)
{
	কাঠামো content *c = (কাঠামো content *)html;
	textarea_mouse_status ta_status;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	নির্বাচন (mas->gadget.control->type) {
	ক্ষেত্র GADGET_SELECT:
		mas->ফলাফল.status = messages_get("FormSelect");
		mas->ফলাফল.pointer = BROWSER_POINTER_MENU;
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
				html->visible_select_menu = ফাঁকা;
			}
			mas->ফলাফল.pointer = BROWSER_POINTER_DEFAULT;
		} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			বার্তা_তথ্য.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_বার্তা_SELECTMENU,
					  &বার্তা_তথ্য);
		}
		বিরতি;

	ক্ষেত্র GADGET_CHECKBOX:
		mas->ফলাফল.status = messages_get("FormCheckbox");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_প্রবেশ_element_set_checked(
				(dom_html_প্রবেশ_element *)(mas->gadget.control->গ্রন্থি),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		বিরতি;

	ক্ষেত্র GADGET_RADIO:
		mas->ফলাফল.status = messages_get("FormRadio");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		বিরতি;

	ক্ষেত্র GADGET_IMAGE:
		/* This falls through to SUBMIT */
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			কাঠামো image_প্রবেশ_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, আকার(*coords));
			যদি (coords == ফাঁকা) {
				ফেরত NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			যদি (dom_গ্রন্থি_set_user_তথ্য(
				mas->gadget.control->গ্রন্থি,
				corestring_dom_ns_চাবি_image_coords_গ্রন্থি_তথ্য,
				coords,
				html_image_coords_dom_user_তথ্য_handler,
				&oldcoords) != DOM_NO_ERR) {
				ফেরত NSERROR_OK;
			}
			মুক্ত(oldcoords);
		}
		fallthrough;

	ক্ষেত্র GADGET_SUBMIT:
		যদি (mas->gadget.control->form) {
			স্থির অক্ষর status_বাফার[200];

			snprintf(status_বাফার,
				 আকার status_বাফার,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->ফলাফল.status = status_বাফার;
			mas->ফলাফল.pointer = get_pointer_shape(mas->gadget.box,
								false);
			যদি (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->ফলাফল.action = ACTION_SUBMIT;
			}
		} অন্যথা {
			mas->ফলাফল.status = messages_get("FormBadSubmit");
		}
		বিরতি;

	ক্ষেত্র GADGET_TEXTBOX:
	ক্ষেত্র GADGET_PASSWORD:
	ক্ষেত্র GADGET_TEXTAREA:
		যদি (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->ফলাফল.status = messages_get("FormTextarea");
		} অন্যথা {
			mas->ফলাফল.status = messages_get("FormTextbox");
		}

		যদি (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			সংঘ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_set_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_status = textarea_mouse_action(mas->gadget.control->তথ্য.text.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		যদি (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->ফলাফল.pointer = get_pointer_shape(mas->gadget.box, false);
		} অন্যথা {
			mas->ফলাফল.pointer = BROWSER_POINTER_DEFAULT;
			mas->ফলাফল.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		বিরতি;

	ক্ষেত্র GADGET_HIDDEN:
		/* not possible: no box generated */
		বিরতি;

	ক্ষেত্র GADGET_RESET:
		mas->ফলাফল.status = messages_get("FormReset");
		বিরতি;

	ক্ষেত্র GADGET_FILE:
		mas->ফলাফল.status = messages_get("FormFile");
		যদি (mouse & BROWSER_MOUSE_CLICK_1) {
			বার্তা_তথ্য.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_বার্তা_GADGETCLICK,
					  &বার্তা_তথ্য);
		}
		বিরতি;

	ক্ষেত্র GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->ফলাফল.status = messages_get("FormButton");
		বিরতি;
	}

	ফেরত NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
স্থির nserror
iframe_mouse_action(কাঠামো browser_window *bw,
		    browser_mouse_state mouse,
		    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		    কাঠামো mouse_action_state *mas)
{
	পূর্ণসংখ্যা pos_x, pos_y;
	দশমিক scale;

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
	mas->ফলাফল.action = ACTION_NOSEND;

	ফেরত NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
স্থির nserror
html_object_mouse_action(html_content *html,
			 কাঠামো browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			 কাঠামো mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	যদি (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		সংঘ html_selection_owner sel_owner;
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

	mas->ফলাফল.action = ACTION_NOSEND;
	ফেরত NSERROR_OK;
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
	যদি (scheme != ফাঁকা) {
		যদি (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ফেরত is_js;
}


/**
 * process mouse activity on a link
 */
স্থির nserror
link_mouse_action(html_content *html,
		  কাঠামো browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		  কাঠামো mouse_action_state *mas)
{
	nserror res;
	অক্ষর *url_s = ফাঁকা;
	size_t url_l = 0;
	স্থির অক্ষর status_বাফার[200];
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

	যদি (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		যদি (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ফাঁকা;
		}
	}

	যদি (mas->title) {
		snprintf(status_বাফার,
			 আকার status_বাফার,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} অন্যথা {
		snprintf(status_বাফার,
			 আকার status_বাফার,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	যদি (url_s != ফাঁকা) {
		মুক্ত(url_s);
	}

	mas->ফলাফল.status = status_বাফার;

	mas->ফলাফল.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	যদি (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((কাঠামো content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ফাঁকা,
					ফাঁকা,
					ফাঁকা);

	} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		বার্তা_তথ্য.savelink.url = mas->link.url;
		বার্তা_তথ্য.savelink.title = mas->title;
		content_broadcast((কাঠামো content *)html,
				  CONTENT_বার্তা_SAVELINK,
				  &বার্তা_তথ্য);

	} অন্যথা যদি (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		যদি (is_javascript_navigate_url(mas->link.url)) {
			mas->ফলাফল.action = ACTION_JS;
		} অন্যথা {
			mas->ফলাফল.action = ACTION_NAVIGATE;
		}
	}

	ফেরত NSERROR_OK;
}


স্থির nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	যদি (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		সংঘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ফাঁকা);
	}

	ফেরত NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
স্থির nserror
default_mouse_action(html_content *html,
		  কাঠামো browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		  কাঠামো mouse_action_state *mas)
{
	কাঠামো content *c = (কাঠামো content *)html;

	/* frame resizing */
	যদি (browser_window_frame_resize_start(bw, mouse, x, y, &mas->ফলাফল.pointer)) {
		যদি (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->ফলাফল.status = messages_get("FrameDrag");
		}
		ফেরত default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	সংঘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	যদি (click && html->focus_type != HTML_FOCUS_SELF) {
		সংঘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ফাঁকা);
	}
	যদি (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	যদি (mas->text.box) {
		পূর্ণসংখ্যা pixel_offset;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->text.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->text.box->text,
				       mas->text.box->দৈর্ঘ্য,
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
			সংঘ html_drag_owner drag_owner;

			যদি (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   ফাঁকা);
				mas->ফলাফল.status = messages_get("Selecting");
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

			ফেরত default_mouse_action_focus(html, mouse);
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
		mas->ফলাফল.status = mas->title;
	}

	যদি (mouse & BROWSER_MOUSE_DRAG_1) {
		যদি (mouse & BROWSER_MOUSE_MOD_2) {
			সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
			বার্তা_তথ্য.dragsave.type = CONTENT_SAVE_COMPLETE;
			বার্তা_তথ্য.dragsave.content = ফাঁকা;
			content_broadcast(c, CONTENT_বার্তা_DRAGSAVE, &বার্তা_তথ্য);
		} অন্যথা {
			যদি (mas->drag_candidate == ফাঁকা) {
				browser_window_page_drag_start(bw, x, y);
			} অন্যথা {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->ফলাফল.pointer = BROWSER_POINTER_MOVE;
		}
	} অন্যথা যদি (mouse & BROWSER_MOUSE_DRAG_2) {
		যদি (mouse & BROWSER_MOUSE_MOD_2) {
			সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
			বার্তা_তথ্য.dragsave.type = CONTENT_SAVE_SOURCE;
			বার্তা_তথ্য.dragsave.content = ফাঁকা;
			content_broadcast(c, CONTENT_বার্তা_DRAGSAVE, &বার্তা_তথ্য);
		} অন্যথা {
			যদি (mas->drag_candidate == ফাঁকা) {
				browser_window_page_drag_start(bw, x, y);
			} অন্যথা {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->ফলাফল.pointer = BROWSER_POINTER_MOVE;
		}
	}


	ফেরত default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
স্থির nserror
mouse_action_drag_none(html_content *html,
		       কাঠামো browser_window *bw,
		       browser_mouse_state mouse,
		       পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	nserror res;
	কাঠামো content *c = (কাঠামো content *)html;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	স্থির কাঠামো mouse_action_state mas;

	res = get_mouse_action_গ্রন্থি(html, x, y, &mas);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	যদি (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.ফলাফল.action = ACTION_BACK;
	} অন্যথা যদি (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.ফলাফল.action = ACTION_FORWARD;
	} অন্যথা যদি (mas.scroll.bar) {
		mas.ফলাফল.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.ফলাফল.pointer = BROWSER_POINTER_DEFAULT;

	} অন্যথা যদি (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} অন্যথা যদি ((mas.object != ফাঁকা) && (mouse & BROWSER_MOUSE_MOD_2)) {

		যদি (mouse & BROWSER_MOUSE_DRAG_2) {
			বার্তা_তথ্য.dragsave.type = CONTENT_SAVE_NATIVE;
			বার্তা_তথ্য.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_বার্তা_DRAGSAVE, &বার্তা_তথ্য);

		} অন্যথা যদি (mouse & BROWSER_MOUSE_DRAG_1) {
			বার্তা_তথ্য.dragsave.type = CONTENT_SAVE_ORIG;
			বার্তা_তথ্য.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_বার্তা_DRAGSAVE, &বার্তা_তথ্য);
		}

		/* \todo should have a drag-saving object msg */

	} অন্যথা যদি (mas.iframe != ফাঁকা) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} অন্যথা যদি (mas.html_object.box != ফাঁকা) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} অন্যথা যদি (mas.link.url != ফাঁকা) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} অন্যথা {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	/* send status and pointer message */
	যদি (mas.ফলাফল.action != ACTION_NOSEND) {
		বার্তা_তথ্য.explicit_status_text = mas.ফলাফল.status;
		content_broadcast(c, CONTENT_বার্তা_STATUS, &বার্তা_তথ্য);

		বার্তা_তথ্য.pointer = mas.ফলাফল.pointer;
		content_broadcast(c, CONTENT_বার্তা_POINTER, &বার্তা_তথ্য);
	}

	/* fire dom click event */
	যদি (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.গ্রন্থি, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	নির্বাচন (mas.ফলাফল.action) {
	ক্ষেত্র ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		বিরতি;

	ক্ষেত্র ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ফাঁকা,
				ফাঁকা,
				ফাঁকা);
		বিরতি;

	ক্ষেত্র ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		যদি (path != ফাঁকা) {
			html_exec(c,
				  lwc_string_তথ্য(path),
				  lwc_string_দৈর্ঘ্য(path));
			lwc_string_unref(path);
		}
		বিরতি;

	ক্ষেত্র ACTION_BACK:
		res = browser_window_history_back(bw, false);
		বিরতি;

	ক্ষেত্র ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		বিরতি;

	ক্ষেত্র ACTION_NOSEND:
	ক্ষেত্র ACTION_NONE:
		res = NSERROR_OK;
		বিরতি;
	}

	ফেরত res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(কাঠামো content *c,
			 কাঠামো browser_window *bw,
			 browser_mouse_state mouse,
			 পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	ফেরত html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(কাঠামো content *c,
		  কাঠামো browser_window *bw,
		  browser_mouse_state mouse,
		  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	যদি (html->visible_select_menu != ফাঁকা) {
		ফেরত mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	নির্বাচন (html->drag_type) {
	ক্ষেত্র HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		বিরতি;

	ক্ষেত্র HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		বিরতি;

	ক্ষেত্র HTML_DRAG_TEXTAREA_SELECTION:
	ক্ষেত্র HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		বিরতি;

	ক্ষেত্র HTML_DRAG_CONTENT_SELECTION:
	ক্ষেত্র HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		বিরতি;

	ক্ষেত্র HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		বিরতি;

	পূর্বনির্ধারিত:
		/* Unknown content related drag type */
		প্রতিজ্ঞা(0 && "Unknown content related drag type");
	}

	যদি (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ফেরত res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(কাঠামো content *c, uint32_t চাবি)
{
	html_content *html = (html_content *) c;
	কাঠামো selection *sel = html->sel;

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
	যদি (html->layout != ফাঁকা && html->layout->গ্রন্থি != ফাঁকা) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->গ্রন্থি, true, true, চাবি);
	}

	নির্বাচন (html->focus_type) {
	ক্ষেত্র HTML_FOCUS_CONTENT:
		ফেরত content_keypress(html->focus_owner.content->object, চাবি);

	ক্ষেত্র HTML_FOCUS_TEXTAREA:
		যদি (box_textarea_keypress(html, html->focus_owner.textarea, চাবি) == NSERROR_OK) {
			ফেরত true;
		} অন্যথা {
			ফেরত false;
		}

	পূর্বনির্ধারিত:
		/* Deal with it below */
		বিরতি;
	}

	নির্বাচন (চাবি) {
	ক্ষেত্র NS_চাবি_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ফেরত true;

	ক্ষেত্র NS_চাবি_CLEAR_SELECTION:
		selection_clear(sel, true);
		ফেরত true;

	ক্ষেত্র NS_চাবি_SELECT_ALL:
		selection_select_all(sel);
		ফেরত true;

	ক্ষেত্র NS_চাবি_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ফেরত selection_clear(sel, true);
	}

	ফেরত false;
}


/**
 * Callback for in-page scrollbars.
 */
শূন্য html_overflow_scroll_callback(শূন্য *client_তথ্য,
		কাঠামো scrollbar_বার্তা_তথ্য *scrollbar_তথ্য)
{
	কাঠামো html_scrollbar_তথ্য *তথ্য = client_তথ্য;
	html_content *html = (html_content *)তথ্য->c;
	কাঠামো box *box = তথ্য->box;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	html_drag_type drag_type;
	সংঘ html_drag_owner drag_owner;

	নির্বাচন(scrollbar_তথ্য->বার্তা) {
	ক্ষেত্র SCROLLBAR_বার্তা_MOVED:

		যদি (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			বিরতি;
		}

		html__redraw_a_box(html, box);
		বিরতি;
	ক্ষেত্র SCROLLBAR_বার্তা_SCROLL_START:
	{
		কাঠামো rect rect = {
			.x0 = scrollbar_তথ্য->x0,
			.y0 = scrollbar_তথ্য->y0,
			.x1 = scrollbar_তথ্য->x1,
			.y1 = scrollbar_তথ্য->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_তথ্য->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		বিরতি;
	ক্ষেত্র SCROLLBAR_বার্তা_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, ফাঁকা);

		বার্তা_তথ্য.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(তথ্য->c, CONTENT_বার্তা_POINTER, &বার্তা_তথ্য);
		বিরতি;
	}
}


/* Documented in html_internal.h */
শূন্য html_set_drag_type(html_content *html, html_drag_type drag_type,
		সংঘ html_drag_owner drag_owner, ধ্রুবক কাঠামো rect *rect)
{
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

	প্রতিজ্ঞা(html != ফাঁকা);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	নির্বাচন (drag_type) {
	ক্ষেত্র HTML_DRAG_NONE:
		প্রতিজ্ঞা(drag_owner.no_owner == true);
		বার্তা_তথ্য.drag.type = CONTENT_DRAG_NONE;
		বিরতি;

	ক্ষেত্র HTML_DRAG_SCROLLBAR:
	ক্ষেত্র HTML_DRAG_TEXTAREA_SCROLLBAR:
	ক্ষেত্র HTML_DRAG_CONTENT_SCROLL:
		বার্তা_তথ্য.drag.type = CONTENT_DRAG_SCROLL;
		বিরতি;

	ক্ষেত্র HTML_DRAG_SELECTION:
		প্রতিজ্ঞা(drag_owner.no_owner == true);
		fallthrough;
	ক্ষেত্র HTML_DRAG_TEXTAREA_SELECTION:
	ক্ষেত্র HTML_DRAG_CONTENT_SELECTION:
		বার্তা_তথ্য.drag.type = CONTENT_DRAG_SELECTION;
		বিরতি;
	}
	বার্তা_তথ্য.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((কাঠামো content *)html, CONTENT_বার্তা_DRAG, &বার্তা_তথ্য);
}

/* Documented in html_internal.h */
শূন্য html_set_focus(html_content *html, html_focus_type focus_type,
		সংঘ html_focus_owner focus_owner, bool hide_caret,
		পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা height, ধ্রুবক কাঠামো rect *clip)
{
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	পূর্ণসংখ্যা x_off = 0;
	পূর্ণসংখ্যা y_off = 0;
	কাঠামো rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	প্রতিজ্ঞা(html != ফাঁকা);

	নির্বাচন (focus_type) {
	ক্ষেত্র HTML_FOCUS_SELF:
		প্রতিজ্ঞা(focus_owner.self == true);
		যদি (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ফেরত;
		বিরতি;

	ক্ষেত্র HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		বিরতি;

	ক্ষেত্র HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		বিরতি;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	যদি (textarea_lost_focus) {
		বার্তা_তথ্য.caret.type = CONTENT_CARET_REMOVE;
	} অন্যথা যদি (focus_type != HTML_FOCUS_SELF && hide_caret) {
		বার্তা_তথ্য.caret.type = CONTENT_CARET_HIDE;
	} অন্যথা {
		যদি (clip != ফাঁকা) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		বার্তা_তথ্য.caret.type = CONTENT_CARET_SET_POS;
		বার্তা_তথ্য.caret.pos.x = x + x_off;
		বার্তা_তথ্য.caret.pos.y = y + y_off;
		বার্তা_তথ্য.caret.pos.height = height;
		বার্তা_তথ্য.caret.pos.clip = (clip == ফাঁকা) ? ফাঁকা : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((কাঠামো content *)html, CONTENT_বার্তা_CARET, &বার্তা_তথ্য);
}

/* Documented in html_internal.h */
শূন্য html_set_selection(html_content *html, html_selection_type selection_type,
		সংঘ html_selection_owner selection_owner, bool read_only)
{
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	কাঠামো box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	প্রতিজ্ঞা(html != ফাঁকা);

	যদি ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	যদি (html->selection_type != HTML_SELECTION_NONE) {
		নির্বাচন (html->selection_type) {
		ক্ষেত্র HTML_SELECTION_SELF:
			যদি (same_type)
				বিরতি;
			selection_clear(html->sel, true);
			বিরতি;
		ক্ষেত্র HTML_SELECTION_TEXTAREA:
			যদি (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				বিরতি;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->তথ্য.text.ta);
			বিরতি;
		ক্ষেত্র HTML_SELECTION_CONTENT:
			যদি (same_type && html->selection_owner.content ==
					selection_owner.content)
				বিরতি;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			বিরতি;
		পূর্বনির্ধারিত:
			বিরতি;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	যদি (!changed)
		/* Don't need to report lack of change to owner */
		ফেরত;

	/* Prepare msg */
	নির্বাচন (selection_type) {
	ক্ষেত্র HTML_SELECTION_NONE:
		প্রতিজ্ঞা(selection_owner.none == true);
		বার্তা_তথ্য.selection.selection = false;
		বিরতি;
	ক্ষেত্র HTML_SELECTION_SELF:
		প্রতিজ্ঞা(selection_owner.none == false);
		fallthrough;
	ক্ষেত্র HTML_SELECTION_TEXTAREA:
	ক্ষেত্র HTML_SELECTION_CONTENT:
		বার্তা_তথ্য.selection.selection = true;
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}
	বার্তা_তথ্য.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((কাঠামো content *)html, CONTENT_বার্তা_SELECTION,
			&বার্তা_তথ্য);
}
