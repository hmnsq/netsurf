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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/mouse.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "netsurf/layout.h"
#அந்தர்பாவ "netsurf/keypress.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "content/textsearch.h"
#அந்தர்பாவ "desktop/browser_history.h"
#அந்தர்பாவ "desktop/frames.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/selection.h"
#அந்தர்பாவ "desktop/textarea.h"
#அந்தர்பாவ "javascript/js.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_textarea.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/imagemap.h"
#அந்தர்பாவ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

நிலையான browser_pointer_shape get_pointer_shape(கட்டமைப்பு box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	எண்ணிக்கை css_cursor_e cursor;
	lwc_string **cursor_uris;

	எனில் (box->type == BOX_FLOAT_இடது || box->type == BOX_FLOAT_வலது)
		style = box->children->style;
	இல்லையெனில்
		style = box->style;

	எனில் (style == வெற்று)
		திரும்பு BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	தேர்வு (cursor) {
	நிலை CSS_CURSOR_AUTO:
		எனில் (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} இல்லையெனில் எனில் (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} இல்லையெனில் {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		நிறுத்து;
	நிலை CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		நிறுத்து;
	நிலை CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		நிறுத்து;
	நிலை CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		நிறுத்து;
	நிலை CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_வலது;
		நிறுத்து;
	நிலை CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_இடது;
		நிறுத்து;
	நிலை CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		நிறுத்து;
	நிலை CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		நிறுத்து;
	நிலை CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		நிறுத்து;
	நிலை CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		நிறுத்து;
	நிலை CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		நிறுத்து;
	நிலை CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		நிறுத்து;
	நிலை CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		நிறுத்து;
	நிலை CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		நிறுத்து;
	நிலை CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		நிறுத்து;
	நிலை CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		நிறுத்து;
	இயல்பு:
		pointer = BROWSER_POINTER_DEFAULT;
		நிறுத்து;
	}

	திரும்பு pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

நிலையான வெறுமை html_box_drag_start(கட்டமைப்பு box *box, முழு x, முழு y)
{
	முழு box_x, box_y;
	முழு scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	எனில் (box->scroll_x != வெற்று) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[உச்சி_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} இல்லையெனில் எனில் (box->scroll_y != வெற்று) {
		scroll_mouse_x = x - (box_x + box->padding[இடது_2] +
				box->width + box->padding[வலது_2] -
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
நிலையான size_t html_selection_drag_end(கட்டமைப்பு html_content *html,
		browser_mouse_state mouse, முழு x, முழு y, முழு dir)
{
	முழு pixel_offset;
	கட்டமைப்பு box *box;
	முழு dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	எனில் (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	திரும்பு idx;
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
நிலையான வெறுமை
html__image_coords_dom_user_data_handler(dom_முனை_operation operation,
					 dom_string *சாவி,
					 வெறுமை *_data,
					 கட்டமைப்பு dom_முனை *src,
					 கட்டமைப்பு dom_முனை *dst)
{
	கட்டமைப்பு image_உள்ளீடு_coords *oldcoords, *coords = _data, *newcoords;

	எனில் (!dom_string_isequal(corestring_dom_ns_சாவி_image_coords_முனை_data,
				சாவி) || coords == வெற்று) {
		திரும்பு;
	}

	தேர்வு (operation) {
	நிலை DOM_முனை_CLONED:
		newcoords = calloc(1, அளவு(*newcoords));
		எனில் (newcoords != வெற்று) {
			*newcoords = *coords;
			எனில் (dom_முனை_set_user_data(dst,
				 corestring_dom_ns_சாவி_image_coords_முனை_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				விடுவி(oldcoords);
			}
		}
		நிறுத்து;

	நிலை DOM_முனை_DELETED:
		விடுவி(coords);
		நிறுத்து;

	நிலை DOM_முனை_RENAMED:
	நிலை DOM_முனை_IMPORTED:
	நிலை DOM_முனை_ADOPTED:
		நிறுத்து;

	இயல்பு:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ப்ரதிஜ்ஞா(0);
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
நிலையான வெறுமை
html_overflow_scroll_drag_end(கட்டமைப்பு scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      முழு x, முழு y)
{
	முழு scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	கட்டமைப்பு html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	கட்டமைப்பு box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	எனில் (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[உச்சி_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} இல்லையெனில் {
		scroll_mouse_x = x - (box_x + box->padding[இடது_2] +
				box->width + box->padding[வலது_2] -
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
நிலையான nserror
mouse_action_select_menu(html_content *html,
			 கட்டமைப்பு browser_window *bw,
			 browser_mouse_state mouse,
			 முழு x, முழு y)
{
	கட்டமைப்பு box *box;
	முழு box_x = 0;
	முழு box_y = 0;
	மாறிலி எழுத்து *status;
	முழு width, height;
	கட்டமைப்பு hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ப்ரதிஜ்ஞா(html->visible_select_menu != வெற்று);

	bw_drag_type = browser_window_get_drag_type(bw);
	எனில் (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[இடது_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[உச்சி_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	எனில் (status != வெற்று) {
		/* set status if menu still open */
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;
		செய்தி_data.explicit_status_text = status;
		content_broadcast((கட்டமைப்பு content *)html,
				  CONTENT_செய்தி_STATUS,
				  &செய்தி_data);
		திரும்பு NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = வெற்று;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	திரும்பு NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
நிலையான nserror
mouse_action_drag_selection(html_content *html,
			    கட்டமைப்பு browser_window *bw,
			    browser_mouse_state mouse,
			    முழு x, முழு y)
{
	கட்டமைப்பு box *box;
	முழு dir = -1;
	முழு dx, dy;
	size_t idx;
	ஒன்றிப்பு html_drag_owner drag_owner;
	முழு pixel_offset;
	plot_font_style_t fstyle;

	எனில் (!mouse) {
		/* End of selection drag */
		எனில் (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		எனில் (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, வெற்று);

		திரும்பு NSERROR_OK;
	}

	எனில் (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	எனில் (box != வெற்று) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	திரும்பு NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
நிலையான nserror
mouse_action_drag_scrollbar(html_content *html,
			    கட்டமைப்பு browser_window *bw,
			    browser_mouse_state mouse,
			    முழு x, முழு y)
{
	கட்டமைப்பு scrollbar *scr;
	கட்டமைப்பு html_scrollbar_data *data;
	கட்டமைப்பு box *box;
	முழு box_x = 0;
	முழு box_y = 0;
	மாறிலி எழுத்து *status;
	முழு scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	எனில் (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	எனில் (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[உச்சி_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} இல்லையெனில் {
		scroll_mouse_x = x - (box_x + box->padding[இடது_2] +
				      box->width + box->padding[வலது_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	எனில் (status != வெற்று) {
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;

		செய்தி_data.explicit_status_text = status;
		content_broadcast((கட்டமைப்பு content *)html,
				  CONTENT_செய்தி_STATUS,
				  &செய்தி_data);
	}

	திரும்பு NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
நிலையான nserror
mouse_action_drag_textarea(html_content *html,
			    கட்டமைப்பு browser_window *bw,
			    browser_mouse_state mouse,
			    முழு x, முழு y)
{
	கட்டமைப்பு box *box;
	முழு box_x = 0;
	முழு box_y = 0;

	box = html->drag_owner.textarea;

	ப்ரதிஜ்ஞா(box->gadget != வெற்று);
	ப்ரதிஜ்ஞா(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	திரும்பு NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
நிலையான nserror
mouse_action_drag_content(html_content *html,
			  கட்டமைப்பு browser_window *bw,
			  browser_mouse_state mouse,
			  முழு x, முழு y)
{
	கட்டமைப்பு box *box;
	முழு box_x = 0;
	முழு box_y = 0;

	box = html->drag_owner.content;
	ப்ரதிஜ்ஞா(box->object != வெற்று);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	திரும்பு NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
கட்டமைப்பு mouse_action_state {
	கட்டமைப்பு {
		மாறிலி எழுத்து *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		எண்ணிக்கை {
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
	கட்டமைப்பு dom_முனை *முனை;

	/** html object */
	கட்டமைப்பு {
		கட்டமைப்பு box *box;
		முழு pos_x;
		முழு pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	கட்டமைப்பு browser_window *iframe;

	/** link either from href or imagemap */
	கட்டமைப்பு {
		கட்டமைப்பு box *box;
		nsurl *url;
		மாறிலி எழுத்து *target;
		bool is_imagemap;
	} link;

	/** gadget */
	கட்டமைப்பு {
		கட்டமைப்பு form_control *control;
		கட்டமைப்பு box *box;
		முழு box_x;
		முழு box_y;
		மாறிலி எழுத்து *target;
	} gadget;

	/** title */
	மாறிலி எழுத்து *title;

	/** candidate box for drag operation */
	கட்டமைப்பு box *drag_candidate;

	/** scrollbar */
	கட்டமைப்பு {
		கட்டமைப்பு scrollbar *bar;
		முழு mouse_x;
		முழு mouse_y;
	} scroll;

	/** text in box */
	கட்டமைப்பு {
		கட்டமைப்பு box *box;
		முழு box_x;
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
நிலையான nserror
get_mouse_action_முனை(html_content *html,
		      முழு x, முழு y,
		      கட்டமைப்பு mouse_action_state *man)
{
	கட்டமைப்பு box *box;
	முழு box_x = 0;
	முழு box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, அளவு(கட்டமைப்பு mouse_action_state));
	man->முனை = html->layout->முனை; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[இடது_2];
	box_y = box->margin[உச்சி_2];

	செய் {
		/* skip hidden boxes */
		எனில் ((box->style != வெற்று) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			செல் next_box;
		}

		எனில் (box->முனை != வெற்று) {
			man->முனை = box->முனை;
		}

		எனில் (box->object) {
			எனில் (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} இல்லையெனில் {
				man->object = box->object;
			}
		}

		எனில் (box->iframe) {
			man->iframe = box->iframe;
		}

		எனில் (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		எனில் (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		எனில் (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			எனில் (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		எனில் (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		எனில் ((box->scroll_x != வெற்று) ||
		    (box->scroll_y != வெற்று)) {
			முழு padding_இடது;
			முழு padding_வலது;
			முழு padding_உச்சி;
			முழு padding_bottom;

			எனில் (man->drag_candidate == வெற்று) {
				man->drag_candidate = box;
			}

			padding_இடது = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_வலது = padding_இடது + box->padding[இடது_2] +
					box->width + box->padding[வலது_2];
			padding_உச்சி = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_உச்சி + box->padding[உச்சி_2] +
					box->height + box->padding[BOTTOM];

			எனில் ((x > padding_இடது) &&
			    (x < padding_வலது) &&
			    (y > padding_உச்சி) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				எனில் ((box->scroll_y != வெற்று) &&
				    (x > (padding_வலது - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_வலது - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_உச்சி;
					நிறுத்து;

				} இல்லையெனில் எனில் ((box->scroll_x != வெற்று) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_இடது;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					நிறுத்து;
				}
			}
		}

		எனில் (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} போது (box != வெற்று);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ப்ரதிஜ்ஞா(man->முனை != வெற்று);

	திரும்பு NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
நிலையான nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    முழு x, முழு y,
		    கட்டமைப்பு mouse_action_state *mas)
{
	கட்டமைப்பு content *c = (கட்டமைப்பு content *)html;
	textarea_mouse_status ta_status;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	தேர்வு (mas->gadget.control->type) {
	நிலை GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		எனில் (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			எனில் (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = வெற்று;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
			செய்தி_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_செய்தி_SELECTMENU,
					  &செய்தி_data);
		}
		நிறுத்து;

	நிலை GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_உள்ளீடு_element_set_checked(
				(dom_html_உள்ளீடு_element *)(mas->gadget.control->முனை),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		நிறுத்து;

	நிலை GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		நிறுத்து;

	நிலை GADGET_IMAGE:
		/* This falls through to SUBMIT */
		எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
			கட்டமைப்பு image_உள்ளீடு_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, அளவு(*coords));
			எனில் (coords == வெற்று) {
				திரும்பு NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			எனில் (dom_முனை_set_user_data(
				mas->gadget.control->முனை,
				corestring_dom_ns_சாவி_image_coords_முனை_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				திரும்பு NSERROR_OK;
			}
			விடுவி(oldcoords);
		}
		fallthrough;

	நிலை GADGET_SUBMIT:
		எனில் (mas->gadget.control->form) {
			நிலையான எழுத்து status_buffer[200];

			snprintf(status_buffer,
				 அளவு status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			எனில் (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} இல்லையெனில் {
			mas->result.status = messages_get("FormBadSubmit");
		}
		நிறுத்து;

	நிலை GADGET_TEXTBOX:
	நிலை GADGET_PASSWORD:
	நிலை GADGET_TEXTAREA:
		எனில் (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} இல்லையெனில் {
			mas->result.status = messages_get("FormTextbox");
		}

		எனில் (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			ஒன்றிப்பு html_selection_owner sel_owner;
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

		எனில் (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} இல்லையெனில் {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		நிறுத்து;

	நிலை GADGET_HIDDEN:
		/* not possible: no box generated */
		நிறுத்து;

	நிலை GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		நிறுத்து;

	நிலை GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
			செய்தி_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_செய்தி_GADGETCLICK,
					  &செய்தி_data);
		}
		நிறுத்து;

	நிலை GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		நிறுத்து;
	}

	திரும்பு NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
நிலையான nserror
iframe_mouse_action(கட்டமைப்பு browser_window *bw,
		    browser_mouse_state mouse,
		    முழு x, முழு y,
		    கட்டமைப்பு mouse_action_state *mas)
{
	முழு pos_x, pos_y;
	தசம scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	எனில் (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} இல்லையெனில் {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	திரும்பு NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
நிலையான nserror
html_object_mouse_action(html_content *html,
			 கட்டமைப்பு browser_window *bw,
			 browser_mouse_state mouse,
			 முழு x, முழு y,
			 கட்டமைப்பு mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	எனில் (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		ஒன்றிப்பு html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	எனில் (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} இல்லையெனில் {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	திரும்பு NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
நிலையான bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	எனில் (scheme != வெற்று) {
		எனில் (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	திரும்பு is_js;
}


/**
 * process mouse activity on a link
 */
நிலையான nserror
link_mouse_action(html_content *html,
		  கட்டமைப்பு browser_window *bw,
		  browser_mouse_state mouse,
		  முழு x, முழு y,
		  கட்டமைப்பு mouse_action_state *mas)
{
	nserror res;
	எழுத்து *url_s = வெற்று;
	size_t url_l = 0;
	நிலையான எழுத்து status_buffer[200];
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;

	எனில் (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		எனில் (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = வெற்று;
		}
	}

	எனில் (mas->title) {
		snprintf(status_buffer,
			 அளவு status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} இல்லையெனில் {
		snprintf(status_buffer,
			 அளவு status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	எனில் (url_s != வெற்று) {
		விடுவி(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	எனில் (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((கட்டமைப்பு content *)html),
					BW_NAVIGATE_DOWNLOAD,
					வெற்று,
					வெற்று,
					வெற்று);

	} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		செய்தி_data.savelink.url = mas->link.url;
		செய்தி_data.savelink.title = mas->title;
		content_broadcast((கட்டமைப்பு content *)html,
				  CONTENT_செய்தி_SAVELINK,
				  &செய்தி_data);

	} இல்லையெனில் எனில் (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		எனில் (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} இல்லையெனில் {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	திரும்பு NSERROR_OK;
}


நிலையான nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	எனில் (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		ஒன்றிப்பு html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, வெற்று);
	}

	திரும்பு NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
நிலையான nserror
default_mouse_action(html_content *html,
		  கட்டமைப்பு browser_window *bw,
		  browser_mouse_state mouse,
		  முழு x, முழு y,
		  கட்டமைப்பு mouse_action_state *mas)
{
	கட்டமைப்பு content *c = (கட்டமைப்பு content *)html;

	/* frame resizing */
	எனில் (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		எனில் (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		திரும்பு default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	ஒன்றிப்பு html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	எனில் (click && html->focus_type != HTML_FOCUS_SELF) {
		ஒன்றிப்பு html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, வெற்று);
	}
	எனில் (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	எனில் (mas->text.box) {
		முழு pixel_offset;
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

		எனில் (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			ஒன்றிப்பு html_drag_owner drag_owner;

			எனில் (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   வெற்று);
				mas->result.status = messages_get("Selecting");
			}

			எனில் (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} இல்லையெனில் எனில் (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			திரும்பு default_mouse_action_focus(html, mouse);
		}

	} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	எனில் (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} இல்லையெனில் எனில் (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	எனில் (mas->title) {
		mas->result.status = mas->title;
	}

	எனில் (mouse & BROWSER_MOUSE_DRAG_1) {
		எனில் (mouse & BROWSER_MOUSE_MOD_2) {
			ஒன்றிப்பு content_செய்தி_data செய்தி_data;
			செய்தி_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			செய்தி_data.dragsave.content = வெற்று;
			content_broadcast(c, CONTENT_செய்தி_DRAGSAVE, &செய்தி_data);
		} இல்லையெனில் {
			எனில் (mas->drag_candidate == வெற்று) {
				browser_window_page_drag_start(bw, x, y);
			} இல்லையெனில் {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_DRAG_2) {
		எனில் (mouse & BROWSER_MOUSE_MOD_2) {
			ஒன்றிப்பு content_செய்தி_data செய்தி_data;
			செய்தி_data.dragsave.type = CONTENT_SAVE_SOURCE;
			செய்தி_data.dragsave.content = வெற்று;
			content_broadcast(c, CONTENT_செய்தி_DRAGSAVE, &செய்தி_data);
		} இல்லையெனில் {
			எனில் (mas->drag_candidate == வெற்று) {
				browser_window_page_drag_start(bw, x, y);
			} இல்லையெனில் {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	திரும்பு default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
நிலையான nserror
mouse_action_drag_none(html_content *html,
		       கட்டமைப்பு browser_window *bw,
		       browser_mouse_state mouse,
		       முழு x, முழு y)
{
	nserror res;
	கட்டமைப்பு content *c = (கட்டமைப்பு content *)html;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	நிலையான கட்டமைப்பு mouse_action_state mas;

	res = get_mouse_action_முனை(html, x, y, &mas);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	எனில் (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} இல்லையெனில் எனில் (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} இல்லையெனில் எனில் (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} இல்லையெனில் எனில் ((mas.object != வெற்று) && (mouse & BROWSER_MOUSE_MOD_2)) {

		எனில் (mouse & BROWSER_MOUSE_DRAG_2) {
			செய்தி_data.dragsave.type = CONTENT_SAVE_NATIVE;
			செய்தி_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_செய்தி_DRAGSAVE, &செய்தி_data);

		} இல்லையெனில் எனில் (mouse & BROWSER_MOUSE_DRAG_1) {
			செய்தி_data.dragsave.type = CONTENT_SAVE_ORIG;
			செய்தி_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_செய்தி_DRAGSAVE, &செய்தி_data);
		}

		/* \todo should have a drag-saving object msg */

	} இல்லையெனில் எனில் (mas.iframe != வெற்று) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} இல்லையெனில் எனில் (mas.html_object.box != வெற்று) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} இல்லையெனில் எனில் (mas.link.url != வெற்று) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} இல்லையெனில் {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	/* send status and pointer message */
	எனில் (mas.result.action != ACTION_NOSEND) {
		செய்தி_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_செய்தி_STATUS, &செய்தி_data);

		செய்தி_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_செய்தி_POINTER, &செய்தி_data);
	}

	/* fire dom click event */
	எனில் (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.முனை, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	தேர்வு (mas.result.action) {
	நிலை ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		நிறுத்து;

	நிலை ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				வெற்று,
				வெற்று,
				வெற்று);
		நிறுத்து;

	நிலை ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		எனில் (path != வெற்று) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		நிறுத்து;

	நிலை ACTION_BACK:
		res = browser_window_history_back(bw, false);
		நிறுத்து;

	நிலை ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		நிறுத்து;

	நிலை ACTION_NOSEND:
	நிலை ACTION_NONE:
		res = NSERROR_OK;
		நிறுத்து;
	}

	திரும்பு res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(கட்டமைப்பு content *c,
			 கட்டமைப்பு browser_window *bw,
			 browser_mouse_state mouse,
			 முழு x, முழு y)
{
	திரும்பு html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(கட்டமைப்பு content *c,
		  கட்டமைப்பு browser_window *bw,
		  browser_mouse_state mouse,
		  முழு x, முழு y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	எனில் (html->visible_select_menu != வெற்று) {
		திரும்பு mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	தேர்வு (html->drag_type) {
	நிலை HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		நிறுத்து;

	நிலை HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		நிறுத்து;

	நிலை HTML_DRAG_TEXTAREA_SELECTION:
	நிலை HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		நிறுத்து;

	நிலை HTML_DRAG_CONTENT_SELECTION:
	நிலை HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		நிறுத்து;

	நிலை HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		நிறுத்து;

	இயல்பு:
		/* Unknown content related drag type */
		ப்ரதிஜ்ஞா(0 && "Unknown content related drag type");
	}

	எனில் (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	திரும்பு res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(கட்டமைப்பு content *c, uint32_t சாவி)
{
	html_content *html = (html_content *) c;
	கட்டமைப்பு selection *sel = html->sel;

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
	எனில் (html->layout != வெற்று && html->layout->முனை != வெற்று) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->முனை, true, true, சாவி);
	}

	தேர்வு (html->focus_type) {
	நிலை HTML_FOCUS_CONTENT:
		திரும்பு content_keypress(html->focus_owner.content->object, சாவி);

	நிலை HTML_FOCUS_TEXTAREA:
		எனில் (box_textarea_keypress(html, html->focus_owner.textarea, சாவி) == NSERROR_OK) {
			திரும்பு true;
		} இல்லையெனில் {
			திரும்பு false;
		}

	இயல்பு:
		/* Deal with it below */
		நிறுத்து;
	}

	தேர்வு (சாவி) {
	நிலை NS_சாவி_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		திரும்பு true;

	நிலை NS_சாவி_CLEAR_SELECTION:
		selection_clear(sel, true);
		திரும்பு true;

	நிலை NS_சாவி_SELECT_ALL:
		selection_select_all(sel);
		திரும்பு true;

	நிலை NS_சாவி_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		திரும்பு selection_clear(sel, true);
	}

	திரும்பு false;
}


/**
 * Callback for in-page scrollbars.
 */
வெறுமை html_overflow_scroll_callback(வெறுமை *client_data,
		கட்டமைப்பு scrollbar_செய்தி_data *scrollbar_data)
{
	கட்டமைப்பு html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	கட்டமைப்பு box *box = data->box;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	html_drag_type drag_type;
	ஒன்றிப்பு html_drag_owner drag_owner;

	தேர்வு(scrollbar_data->செய்தி) {
	நிலை SCROLLBAR_செய்தி_MOVED:

		எனில் (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			நிறுத்து;
		}

		html__redraw_a_box(html, box);
		நிறுத்து;
	நிலை SCROLLBAR_செய்தி_SCROLL_START:
	{
		கட்டமைப்பு rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		நிறுத்து;
	நிலை SCROLLBAR_செய்தி_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, வெற்று);

		செய்தி_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_செய்தி_POINTER, &செய்தி_data);
		நிறுத்து;
	}
}


/* Documented in html_internal.h */
வெறுமை html_set_drag_type(html_content *html, html_drag_type drag_type,
		ஒன்றிப்பு html_drag_owner drag_owner, மாறிலி கட்டமைப்பு rect *rect)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;

	ப்ரதிஜ்ஞா(html != வெற்று);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	தேர்வு (drag_type) {
	நிலை HTML_DRAG_NONE:
		ப்ரதிஜ்ஞா(drag_owner.no_owner == true);
		செய்தி_data.drag.type = CONTENT_DRAG_NONE;
		நிறுத்து;

	நிலை HTML_DRAG_SCROLLBAR:
	நிலை HTML_DRAG_TEXTAREA_SCROLLBAR:
	நிலை HTML_DRAG_CONTENT_SCROLL:
		செய்தி_data.drag.type = CONTENT_DRAG_SCROLL;
		நிறுத்து;

	நிலை HTML_DRAG_SELECTION:
		ப்ரதிஜ்ஞா(drag_owner.no_owner == true);
		fallthrough;
	நிலை HTML_DRAG_TEXTAREA_SELECTION:
	நிலை HTML_DRAG_CONTENT_SELECTION:
		செய்தி_data.drag.type = CONTENT_DRAG_SELECTION;
		நிறுத்து;
	}
	செய்தி_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((கட்டமைப்பு content *)html, CONTENT_செய்தி_DRAG, &செய்தி_data);
}

/* Documented in html_internal.h */
வெறுமை html_set_focus(html_content *html, html_focus_type focus_type,
		ஒன்றிப்பு html_focus_owner focus_owner, bool hide_caret,
		முழு x, முழு y, முழு height, மாறிலி கட்டமைப்பு rect *clip)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	முழு x_off = 0;
	முழு y_off = 0;
	கட்டமைப்பு rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ப்ரதிஜ்ஞா(html != வெற்று);

	தேர்வு (focus_type) {
	நிலை HTML_FOCUS_SELF:
		ப்ரதிஜ்ஞா(focus_owner.self == true);
		எனில் (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			திரும்பு;
		நிறுத்து;

	நிலை HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		நிறுத்து;

	நிலை HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		நிறுத்து;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	எனில் (textarea_lost_focus) {
		செய்தி_data.caret.type = CONTENT_CARET_REMOVE;
	} இல்லையெனில் எனில் (focus_type != HTML_FOCUS_SELF && hide_caret) {
		செய்தி_data.caret.type = CONTENT_CARET_HIDE;
	} இல்லையெனில் {
		எனில் (clip != வெற்று) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		செய்தி_data.caret.type = CONTENT_CARET_SET_POS;
		செய்தி_data.caret.pos.x = x + x_off;
		செய்தி_data.caret.pos.y = y + y_off;
		செய்தி_data.caret.pos.height = height;
		செய்தி_data.caret.pos.clip = (clip == வெற்று) ? வெற்று : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((கட்டமைப்பு content *)html, CONTENT_செய்தி_CARET, &செய்தி_data);
}

/* Documented in html_internal.h */
வெறுமை html_set_selection(html_content *html, html_selection_type selection_type,
		ஒன்றிப்பு html_selection_owner selection_owner, bool read_only)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	கட்டமைப்பு box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	ப்ரதிஜ்ஞா(html != வெற்று);

	எனில் ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	எனில் (html->selection_type != HTML_SELECTION_NONE) {
		தேர்வு (html->selection_type) {
		நிலை HTML_SELECTION_SELF:
			எனில் (same_type)
				நிறுத்து;
			selection_clear(html->sel, true);
			நிறுத்து;
		நிலை HTML_SELECTION_TEXTAREA:
			எனில் (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				நிறுத்து;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			நிறுத்து;
		நிலை HTML_SELECTION_CONTENT:
			எனில் (same_type && html->selection_owner.content ==
					selection_owner.content)
				நிறுத்து;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			நிறுத்து;
		இயல்பு:
			நிறுத்து;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	எனில் (!changed)
		/* Don't need to report lack of change to owner */
		திரும்பு;

	/* Prepare msg */
	தேர்வு (selection_type) {
	நிலை HTML_SELECTION_NONE:
		ப்ரதிஜ்ஞா(selection_owner.none == true);
		செய்தி_data.selection.selection = false;
		நிறுத்து;
	நிலை HTML_SELECTION_SELF:
		ப்ரதிஜ்ஞா(selection_owner.none == false);
		fallthrough;
	நிலை HTML_SELECTION_TEXTAREA:
	நிலை HTML_SELECTION_CONTENT:
		செய்தி_data.selection.selection = true;
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}
	செய்தி_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((கட்டமைப்பு content *)html, CONTENT_செய்தி_SELECTION,
			&செய்தி_data);
}
