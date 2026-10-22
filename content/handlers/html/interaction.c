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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/mouse.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "netsurf/layout.h"
#અન્તર્ભાવ "netsurf/keypress.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "content/textsearch.h"
#અન્તર્ભાવ "desktop/browser_history.h"
#અન્તર્ભાવ "desktop/frames.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/selection.h"
#અન્તર્ભાવ "desktop/textarea.h"
#અન્તર્ભાવ "javascript/js.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_textarea.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/font.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/imagemap.h"
#અન્તર્ભાવ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

સ્થિર browser_pointer_shape get_pointer_shape(સંરચના box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	ગણના css_cursor_e cursor;
	lwc_string **cursor_uris;

	જો (box->type == BOX_FLOAT_ડાબું || box->type == BOX_FLOAT_જમણું)
		style = box->children->style;
	નહીંતર
		style = box->style;

	જો (style == ખાલી)
		પાછા BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	પસંદગી (cursor) {
	સ્થિતિ CSS_CURSOR_AUTO:
		જો (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} નહીંતર જો (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} નહીંતર {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		તોડો;
	સ્થિતિ CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		તોડો;
	સ્થિતિ CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		તોડો;
	સ્થિતિ CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		તોડો;
	સ્થિતિ CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_જમણું;
		તોડો;
	સ્થિતિ CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_ડાબું;
		તોડો;
	સ્થિતિ CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		તોડો;
	સ્થિતિ CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		તોડો;
	સ્થિતિ CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		તોડો;
	સ્થિતિ CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		તોડો;
	સ્થિતિ CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		તોડો;
	સ્થિતિ CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		તોડો;
	સ્થિતિ CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		તોડો;
	સ્થિતિ CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		તોડો;
	સ્થિતિ CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		તોડો;
	સ્થિતિ CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		તોડો;
	મૂળભૂત:
		pointer = BROWSER_POINTER_DEFAULT;
		તોડો;
	}

	પાછા pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

સ્થિર શૂન્ય html_box_drag_start(સંરચના box *box, પૂર્ણાંક x, પૂર્ણાંક y)
{
	પૂર્ણાંક box_x, box_y;
	પૂર્ણાંક scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	જો (box->scroll_x != ખાલી) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ટોચ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} નહીંતર જો (box->scroll_y != ખાલી) {
		scroll_mouse_x = x - (box_x + box->padding[ડાબું_2] +
				box->width + box->padding[જમણું_2] -
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
સ્થિર size_t html_selection_drag_end(સંરચના html_content *html,
		browser_mouse_state mouse, પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક dir)
{
	પૂર્ણાંક pixel_offset;
	સંરચના box *box;
	પૂર્ણાંક dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	જો (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	પાછા idx;
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
સ્થિર શૂન્ય
html__image_coords_dom_user_data_handler(dom_ગાંઠ_operation operation,
					 dom_string *ચાવી,
					 શૂન્ય *_data,
					 સંરચના dom_ગાંઠ *src,
					 સંરચના dom_ગાંઠ *dst)
{
	સંરચના image_આગત_coords *oldcoords, *coords = _data, *newcoords;

	જો (!dom_string_isequal(corestring_dom_ns_ચાવી_image_coords_ગાંઠ_data,
				ચાવી) || coords == ખાલી) {
		પાછા;
	}

	પસંદગી (operation) {
	સ્થિતિ DOM_ગાંઠ_CLONED:
		newcoords = calloc(1, કદ(*newcoords));
		જો (newcoords != ખાલી) {
			*newcoords = *coords;
			જો (dom_ગાંઠ_set_user_data(dst,
				 corestring_dom_ns_ચાવી_image_coords_ગાંઠ_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				મુક્ત(oldcoords);
			}
		}
		તોડો;

	સ્થિતિ DOM_ગાંઠ_DELETED:
		મુક્ત(coords);
		તોડો;

	સ્થિતિ DOM_ગાંઠ_RENAMED:
	સ્થિતિ DOM_ગાંઠ_IMPORTED:
	સ્થિતિ DOM_ગાંઠ_ADOPTED:
		તોડો;

	મૂળભૂત:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		પ્રતિજ્ઞા(0);
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
સ્થિર શૂન્ય
html_overflow_scroll_drag_end(સંરચના scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      પૂર્ણાંક x, પૂર્ણાંક y)
{
	પૂર્ણાંક scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	સંરચના html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	સંરચના box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	જો (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[ટોચ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} નહીંતર {
		scroll_mouse_x = x - (box_x + box->padding[ડાબું_2] +
				box->width + box->padding[જમણું_2] -
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
સ્થિર nserror
mouse_action_select_menu(html_content *html,
			 સંરચના browser_window *bw,
			 browser_mouse_state mouse,
			 પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના box *box;
	પૂર્ણાંક box_x = 0;
	પૂર્ણાંક box_y = 0;
	અચલ અક્ષર *status;
	પૂર્ણાંક width, height;
	સંરચના hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	પ્રતિજ્ઞા(html->visible_select_menu != ખાલી);

	bw_drag_type = browser_window_get_drag_type(bw);
	જો (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ડાબું_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[ટોચ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	જો (status != ખાલી) {
		/* set status if menu still open */
		સંઘ content_સંદેશ_data સંદેશ_data;
		સંદેશ_data.explicit_status_text = status;
		content_broadcast((સંરચના content *)html,
				  CONTENT_સંદેશ_STATUS,
				  &સંદેશ_data);
		પાછા NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = ખાલી;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	પાછા NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
સ્થિર nserror
mouse_action_drag_selection(html_content *html,
			    સંરચના browser_window *bw,
			    browser_mouse_state mouse,
			    પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના box *box;
	પૂર્ણાંક dir = -1;
	પૂર્ણાંક dx, dy;
	size_t idx;
	સંઘ html_drag_owner drag_owner;
	પૂર્ણાંક pixel_offset;
	plot_font_style_t fstyle;

	જો (!mouse) {
		/* End of selection drag */
		જો (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		જો (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, ખાલી);

		પાછા NSERROR_OK;
	}

	જો (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	જો (box != ખાલી) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	પાછા NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
સ્થિર nserror
mouse_action_drag_scrollbar(html_content *html,
			    સંરચના browser_window *bw,
			    browser_mouse_state mouse,
			    પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના scrollbar *scr;
	સંરચના html_scrollbar_data *data;
	સંરચના box *box;
	પૂર્ણાંક box_x = 0;
	પૂર્ણાંક box_y = 0;
	અચલ અક્ષર *status;
	પૂર્ણાંક scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	જો (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	જો (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ટોચ_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} નહીંતર {
		scroll_mouse_x = x - (box_x + box->padding[ડાબું_2] +
				      box->width + box->padding[જમણું_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	જો (status != ખાલી) {
		સંઘ content_સંદેશ_data સંદેશ_data;

		સંદેશ_data.explicit_status_text = status;
		content_broadcast((સંરચના content *)html,
				  CONTENT_સંદેશ_STATUS,
				  &સંદેશ_data);
	}

	પાછા NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
સ્થિર nserror
mouse_action_drag_textarea(html_content *html,
			    સંરચના browser_window *bw,
			    browser_mouse_state mouse,
			    પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના box *box;
	પૂર્ણાંક box_x = 0;
	પૂર્ણાંક box_y = 0;

	box = html->drag_owner.textarea;

	પ્રતિજ્ઞા(box->gadget != ખાલી);
	પ્રતિજ્ઞા(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	પાછા NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
સ્થિર nserror
mouse_action_drag_content(html_content *html,
			  સંરચના browser_window *bw,
			  browser_mouse_state mouse,
			  પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના box *box;
	પૂર્ણાંક box_x = 0;
	પૂર્ણાંક box_y = 0;

	box = html->drag_owner.content;
	પ્રતિજ્ઞા(box->object != ખાલી);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	પાછા NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
સંરચના mouse_action_state {
	સંરચના {
		અચલ અક્ષર *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		ગણના {
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
	સંરચના dom_ગાંઠ *ગાંઠ;

	/** html object */
	સંરચના {
		સંરચના box *box;
		પૂર્ણાંક pos_x;
		પૂર્ણાંક pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	સંરચના browser_window *iframe;

	/** link either from href or imagemap */
	સંરચના {
		સંરચના box *box;
		nsurl *url;
		અચલ અક્ષર *target;
		bool is_imagemap;
	} link;

	/** gadget */
	સંરચના {
		સંરચના form_control *control;
		સંરચના box *box;
		પૂર્ણાંક box_x;
		પૂર્ણાંક box_y;
		અચલ અક્ષર *target;
	} gadget;

	/** title */
	અચલ અક્ષર *title;

	/** candidate box for drag operation */
	સંરચના box *drag_candidate;

	/** scrollbar */
	સંરચના {
		સંરચના scrollbar *bar;
		પૂર્ણાંક mouse_x;
		પૂર્ણાંક mouse_y;
	} scroll;

	/** text in box */
	સંરચના {
		સંરચના box *box;
		પૂર્ણાંક box_x;
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
સ્થિર nserror
get_mouse_action_ગાંઠ(html_content *html,
		      પૂર્ણાંક x, પૂર્ણાંક y,
		      સંરચના mouse_action_state *man)
{
	સંરચના box *box;
	પૂર્ણાંક box_x = 0;
	પૂર્ણાંક box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, કદ(સંરચના mouse_action_state));
	man->ગાંઠ = html->layout->ગાંઠ; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ડાબું_2];
	box_y = box->margin[ટોચ_2];

	કરો {
		/* skip hidden boxes */
		જો ((box->style != ખાલી) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			જાઓ next_box;
		}

		જો (box->ગાંઠ != ખાલી) {
			man->ગાંઠ = box->ગાંઠ;
		}

		જો (box->object) {
			જો (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} નહીંતર {
				man->object = box->object;
			}
		}

		જો (box->iframe) {
			man->iframe = box->iframe;
		}

		જો (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		જો (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		જો (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			જો (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		જો (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		જો ((box->scroll_x != ખાલી) ||
		    (box->scroll_y != ખાલી)) {
			પૂર્ણાંક padding_ડાબું;
			પૂર્ણાંક padding_જમણું;
			પૂર્ણાંક padding_ટોચ;
			પૂર્ણાંક padding_bottom;

			જો (man->drag_candidate == ખાલી) {
				man->drag_candidate = box;
			}

			padding_ડાબું = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_જમણું = padding_ડાબું + box->padding[ડાબું_2] +
					box->width + box->padding[જમણું_2];
			padding_ટોચ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_ટોચ + box->padding[ટોચ_2] +
					box->height + box->padding[BOTTOM];

			જો ((x > padding_ડાબું) &&
			    (x < padding_જમણું) &&
			    (y > padding_ટોચ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				જો ((box->scroll_y != ખાલી) &&
				    (x > (padding_જમણું - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_જમણું - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_ટોચ;
					તોડો;

				} નહીંતર જો ((box->scroll_x != ખાલી) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ડાબું;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					તોડો;
				}
			}
		}

		જો (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} જ્યારે (box != ખાલી);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	પ્રતિજ્ઞા(man->ગાંઠ != ખાલી);

	પાછા NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
સ્થિર nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    પૂર્ણાંક x, પૂર્ણાંક y,
		    સંરચના mouse_action_state *mas)
{
	સંરચના content *c = (સંરચના content *)html;
	textarea_mouse_status ta_status;
	સંઘ content_સંદેશ_data સંદેશ_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	પસંદગી (mas->gadget.control->type) {
	સ્થિતિ GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		જો (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			જો (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ખાલી;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} નહીંતર જો (mouse & BROWSER_MOUSE_CLICK_1) {
			સંદેશ_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_સંદેશ_SELECTMENU,
					  &સંદેશ_data);
		}
		તોડો;

	સ્થિતિ GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		જો (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_આગત_element_set_checked(
				(dom_html_આગત_element *)(mas->gadget.control->ગાંઠ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		તોડો;

	સ્થિતિ GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		જો (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		તોડો;

	સ્થિતિ GADGET_IMAGE:
		/* This falls through to SUBMIT */
		જો (mouse & BROWSER_MOUSE_CLICK_1) {
			સંરચના image_આગત_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, કદ(*coords));
			જો (coords == ખાલી) {
				પાછા NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			જો (dom_ગાંઠ_set_user_data(
				mas->gadget.control->ગાંઠ,
				corestring_dom_ns_ચાવી_image_coords_ગાંઠ_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				પાછા NSERROR_OK;
			}
			મુક્ત(oldcoords);
		}
		fallthrough;

	સ્થિતિ GADGET_SUBMIT:
		જો (mas->gadget.control->form) {
			સ્થિર અક્ષર status_buffer[200];

			snprintf(status_buffer,
				 કદ status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			જો (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} નહીંતર {
			mas->result.status = messages_get("FormBadSubmit");
		}
		તોડો;

	સ્થિતિ GADGET_TEXTBOX:
	સ્થિતિ GADGET_PASSWORD:
	સ્થિતિ GADGET_TEXTAREA:
		જો (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} નહીંતર {
			mas->result.status = messages_get("FormTextbox");
		}

		જો (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			સંઘ html_selection_owner sel_owner;
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

		જો (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} નહીંતર {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		તોડો;

	સ્થિતિ GADGET_HIDDEN:
		/* not possible: no box generated */
		તોડો;

	સ્થિતિ GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		તોડો;

	સ્થિતિ GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		જો (mouse & BROWSER_MOUSE_CLICK_1) {
			સંદેશ_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_સંદેશ_GADGETCLICK,
					  &સંદેશ_data);
		}
		તોડો;

	સ્થિતિ GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		તોડો;
	}

	પાછા NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
સ્થિર nserror
iframe_mouse_action(સંરચના browser_window *bw,
		    browser_mouse_state mouse,
		    પૂર્ણાંક x, પૂર્ણાંક y,
		    સંરચના mouse_action_state *mas)
{
	પૂર્ણાંક pos_x, pos_y;
	દશાંશ scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	જો (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} નહીંતર {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	પાછા NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
સ્થિર nserror
html_object_mouse_action(html_content *html,
			 સંરચના browser_window *bw,
			 browser_mouse_state mouse,
			 પૂર્ણાંક x, પૂર્ણાંક y,
			 સંરચના mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	જો (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		સંઘ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	જો (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} નહીંતર {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	પાછા NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
સ્થિર bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	જો (scheme != ખાલી) {
		જો (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	પાછા is_js;
}


/**
 * process mouse activity on a link
 */
સ્થિર nserror
link_mouse_action(html_content *html,
		  સંરચના browser_window *bw,
		  browser_mouse_state mouse,
		  પૂર્ણાંક x, પૂર્ણાંક y,
		  સંરચના mouse_action_state *mas)
{
	nserror res;
	અક્ષર *url_s = ખાલી;
	size_t url_l = 0;
	સ્થિર અક્ષર status_buffer[200];
	સંઘ content_સંદેશ_data સંદેશ_data;

	જો (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		જો (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ખાલી;
		}
	}

	જો (mas->title) {
		snprintf(status_buffer,
			 કદ status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} નહીંતર {
		snprintf(status_buffer,
			 કદ status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	જો (url_s != ખાલી) {
		મુક્ત(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	જો (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((સંરચના content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ખાલી,
					ખાલી,
					ખાલી);

	} નહીંતર જો (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		સંદેશ_data.savelink.url = mas->link.url;
		સંદેશ_data.savelink.title = mas->title;
		content_broadcast((સંરચના content *)html,
				  CONTENT_સંદેશ_SAVELINK,
				  &સંદેશ_data);

	} નહીંતર જો (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		જો (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} નહીંતર {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	પાછા NSERROR_OK;
}


સ્થિર nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	જો (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		સંઘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ખાલી);
	}

	પાછા NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
સ્થિર nserror
default_mouse_action(html_content *html,
		  સંરચના browser_window *bw,
		  browser_mouse_state mouse,
		  પૂર્ણાંક x, પૂર્ણાંક y,
		  સંરચના mouse_action_state *mas)
{
	સંરચના content *c = (સંરચના content *)html;

	/* frame resizing */
	જો (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		જો (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		પાછા default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	સંઘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	જો (click && html->focus_type != HTML_FOCUS_SELF) {
		સંઘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ખાલી);
	}
	જો (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	જો (mas->text.box) {
		પૂર્ણાંક pixel_offset;
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

		જો (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			સંઘ html_drag_owner drag_owner;

			જો (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   ખાલી);
				mas->result.status = messages_get("Selecting");
			}

			જો (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} નહીંતર જો (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			પાછા default_mouse_action_focus(html, mouse);
		}

	} નહીંતર જો (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	જો (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} નહીંતર જો (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	જો (mas->title) {
		mas->result.status = mas->title;
	}

	જો (mouse & BROWSER_MOUSE_DRAG_1) {
		જો (mouse & BROWSER_MOUSE_MOD_2) {
			સંઘ content_સંદેશ_data સંદેશ_data;
			સંદેશ_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			સંદેશ_data.dragsave.content = ખાલી;
			content_broadcast(c, CONTENT_સંદેશ_DRAGSAVE, &સંદેશ_data);
		} નહીંતર {
			જો (mas->drag_candidate == ખાલી) {
				browser_window_page_drag_start(bw, x, y);
			} નહીંતર {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} નહીંતર જો (mouse & BROWSER_MOUSE_DRAG_2) {
		જો (mouse & BROWSER_MOUSE_MOD_2) {
			સંઘ content_સંદેશ_data સંદેશ_data;
			સંદેશ_data.dragsave.type = CONTENT_SAVE_SOURCE;
			સંદેશ_data.dragsave.content = ખાલી;
			content_broadcast(c, CONTENT_સંદેશ_DRAGSAVE, &સંદેશ_data);
		} નહીંતર {
			જો (mas->drag_candidate == ખાલી) {
				browser_window_page_drag_start(bw, x, y);
			} નહીંતર {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	પાછા default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
સ્થિર nserror
mouse_action_drag_none(html_content *html,
		       સંરચના browser_window *bw,
		       browser_mouse_state mouse,
		       પૂર્ણાંક x, પૂર્ણાંક y)
{
	nserror res;
	સંરચના content *c = (સંરચના content *)html;
	સંઘ content_સંદેશ_data સંદેશ_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	સ્થિર સંરચના mouse_action_state mas;

	res = get_mouse_action_ગાંઠ(html, x, y, &mas);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	જો (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} નહીંતર જો (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} નહીંતર જો (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} નહીંતર જો (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} નહીંતર જો ((mas.object != ખાલી) && (mouse & BROWSER_MOUSE_MOD_2)) {

		જો (mouse & BROWSER_MOUSE_DRAG_2) {
			સંદેશ_data.dragsave.type = CONTENT_SAVE_NATIVE;
			સંદેશ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_સંદેશ_DRAGSAVE, &સંદેશ_data);

		} નહીંતર જો (mouse & BROWSER_MOUSE_DRAG_1) {
			સંદેશ_data.dragsave.type = CONTENT_SAVE_ORIG;
			સંદેશ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_સંદેશ_DRAGSAVE, &સંદેશ_data);
		}

		/* \todo should have a drag-saving object msg */

	} નહીંતર જો (mas.iframe != ખાલી) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} નહીંતર જો (mas.html_object.box != ખાલી) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} નહીંતર જો (mas.link.url != ખાલી) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} નહીંતર {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	/* send status and pointer message */
	જો (mas.result.action != ACTION_NOSEND) {
		સંદેશ_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_સંદેશ_STATUS, &સંદેશ_data);

		સંદેશ_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_સંદેશ_POINTER, &સંદેશ_data);
	}

	/* fire dom click event */
	જો (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ગાંઠ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	પસંદગી (mas.result.action) {
	સ્થિતિ ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		તોડો;

	સ્થિતિ ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ખાલી,
				ખાલી,
				ખાલી);
		તોડો;

	સ્થિતિ ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		જો (path != ખાલી) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		તોડો;

	સ્થિતિ ACTION_BACK:
		res = browser_window_history_back(bw, false);
		તોડો;

	સ્થિતિ ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		તોડો;

	સ્થિતિ ACTION_NOSEND:
	સ્થિતિ ACTION_NONE:
		res = NSERROR_OK;
		તોડો;
	}

	પાછા res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(સંરચના content *c,
			 સંરચના browser_window *bw,
			 browser_mouse_state mouse,
			 પૂર્ણાંક x, પૂર્ણાંક y)
{
	પાછા html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(સંરચના content *c,
		  સંરચના browser_window *bw,
		  browser_mouse_state mouse,
		  પૂર્ણાંક x, પૂર્ણાંક y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	જો (html->visible_select_menu != ખાલી) {
		પાછા mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	પસંદગી (html->drag_type) {
	સ્થિતિ HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		તોડો;

	સ્થિતિ HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		તોડો;

	સ્થિતિ HTML_DRAG_TEXTAREA_SELECTION:
	સ્થિતિ HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		તોડો;

	સ્થિતિ HTML_DRAG_CONTENT_SELECTION:
	સ્થિતિ HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		તોડો;

	સ્થિતિ HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		તોડો;

	મૂળભૂત:
		/* Unknown content related drag type */
		પ્રતિજ્ઞા(0 && "Unknown content related drag type");
	}

	જો (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	પાછા res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(સંરચના content *c, uint32_t ચાવી)
{
	html_content *html = (html_content *) c;
	સંરચના selection *sel = html->sel;

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
	જો (html->layout != ખાલી && html->layout->ગાંઠ != ખાલી) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ગાંઠ, true, true, ચાવી);
	}

	પસંદગી (html->focus_type) {
	સ્થિતિ HTML_FOCUS_CONTENT:
		પાછા content_keypress(html->focus_owner.content->object, ચાવી);

	સ્થિતિ HTML_FOCUS_TEXTAREA:
		જો (box_textarea_keypress(html, html->focus_owner.textarea, ચાવી) == NSERROR_OK) {
			પાછા true;
		} નહીંતર {
			પાછા false;
		}

	મૂળભૂત:
		/* Deal with it below */
		તોડો;
	}

	પસંદગી (ચાવી) {
	સ્થિતિ NS_ચાવી_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		પાછા true;

	સ્થિતિ NS_ચાવી_CLEAR_SELECTION:
		selection_clear(sel, true);
		પાછા true;

	સ્થિતિ NS_ચાવી_SELECT_ALL:
		selection_select_all(sel);
		પાછા true;

	સ્થિતિ NS_ચાવી_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		પાછા selection_clear(sel, true);
	}

	પાછા false;
}


/**
 * Callback for in-page scrollbars.
 */
શૂન્ય html_overflow_scroll_callback(શૂન્ય *client_data,
		સંરચના scrollbar_સંદેશ_data *scrollbar_data)
{
	સંરચના html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	સંરચના box *box = data->box;
	સંઘ content_સંદેશ_data સંદેશ_data;
	html_drag_type drag_type;
	સંઘ html_drag_owner drag_owner;

	પસંદગી(scrollbar_data->સંદેશ) {
	સ્થિતિ SCROLLBAR_સંદેશ_MOVED:

		જો (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			તોડો;
		}

		html__redraw_a_box(html, box);
		તોડો;
	સ્થિતિ SCROLLBAR_સંદેશ_SCROLL_START:
	{
		સંરચના rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		તોડો;
	સ્થિતિ SCROLLBAR_સંદેશ_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, ખાલી);

		સંદેશ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_સંદેશ_POINTER, &સંદેશ_data);
		તોડો;
	}
}


/* Documented in html_internal.h */
શૂન્ય html_set_drag_type(html_content *html, html_drag_type drag_type,
		સંઘ html_drag_owner drag_owner, અચલ સંરચના rect *rect)
{
	સંઘ content_સંદેશ_data સંદેશ_data;

	પ્રતિજ્ઞા(html != ખાલી);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	પસંદગી (drag_type) {
	સ્થિતિ HTML_DRAG_NONE:
		પ્રતિજ્ઞા(drag_owner.no_owner == true);
		સંદેશ_data.drag.type = CONTENT_DRAG_NONE;
		તોડો;

	સ્થિતિ HTML_DRAG_SCROLLBAR:
	સ્થિતિ HTML_DRAG_TEXTAREA_SCROLLBAR:
	સ્થિતિ HTML_DRAG_CONTENT_SCROLL:
		સંદેશ_data.drag.type = CONTENT_DRAG_SCROLL;
		તોડો;

	સ્થિતિ HTML_DRAG_SELECTION:
		પ્રતિજ્ઞા(drag_owner.no_owner == true);
		fallthrough;
	સ્થિતિ HTML_DRAG_TEXTAREA_SELECTION:
	સ્થિતિ HTML_DRAG_CONTENT_SELECTION:
		સંદેશ_data.drag.type = CONTENT_DRAG_SELECTION;
		તોડો;
	}
	સંદેશ_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((સંરચના content *)html, CONTENT_સંદેશ_DRAG, &સંદેશ_data);
}

/* Documented in html_internal.h */
શૂન્ય html_set_focus(html_content *html, html_focus_type focus_type,
		સંઘ html_focus_owner focus_owner, bool hide_caret,
		પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક height, અચલ સંરચના rect *clip)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	પૂર્ણાંક x_off = 0;
	પૂર્ણાંક y_off = 0;
	સંરચના rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	પ્રતિજ્ઞા(html != ખાલી);

	પસંદગી (focus_type) {
	સ્થિતિ HTML_FOCUS_SELF:
		પ્રતિજ્ઞા(focus_owner.self == true);
		જો (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			પાછા;
		તોડો;

	સ્થિતિ HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		તોડો;

	સ્થિતિ HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		તોડો;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	જો (textarea_lost_focus) {
		સંદેશ_data.caret.type = CONTENT_CARET_REMOVE;
	} નહીંતર જો (focus_type != HTML_FOCUS_SELF && hide_caret) {
		સંદેશ_data.caret.type = CONTENT_CARET_HIDE;
	} નહીંતર {
		જો (clip != ખાલી) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		સંદેશ_data.caret.type = CONTENT_CARET_SET_POS;
		સંદેશ_data.caret.pos.x = x + x_off;
		સંદેશ_data.caret.pos.y = y + y_off;
		સંદેશ_data.caret.pos.height = height;
		સંદેશ_data.caret.pos.clip = (clip == ખાલી) ? ખાલી : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((સંરચના content *)html, CONTENT_સંદેશ_CARET, &સંદેશ_data);
}

/* Documented in html_internal.h */
શૂન્ય html_set_selection(html_content *html, html_selection_type selection_type,
		સંઘ html_selection_owner selection_owner, bool read_only)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	સંરચના box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	પ્રતિજ્ઞા(html != ખાલી);

	જો ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	જો (html->selection_type != HTML_SELECTION_NONE) {
		પસંદગી (html->selection_type) {
		સ્થિતિ HTML_SELECTION_SELF:
			જો (same_type)
				તોડો;
			selection_clear(html->sel, true);
			તોડો;
		સ્થિતિ HTML_SELECTION_TEXTAREA:
			જો (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				તોડો;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			તોડો;
		સ્થિતિ HTML_SELECTION_CONTENT:
			જો (same_type && html->selection_owner.content ==
					selection_owner.content)
				તોડો;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			તોડો;
		મૂળભૂત:
			તોડો;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	જો (!changed)
		/* Don't need to report lack of change to owner */
		પાછા;

	/* Prepare msg */
	પસંદગી (selection_type) {
	સ્થિતિ HTML_SELECTION_NONE:
		પ્રતિજ્ઞા(selection_owner.none == true);
		સંદેશ_data.selection.selection = false;
		તોડો;
	સ્થિતિ HTML_SELECTION_SELF:
		પ્રતિજ્ઞા(selection_owner.none == false);
		fallthrough;
	સ્થિતિ HTML_SELECTION_TEXTAREA:
	સ્થિતિ HTML_SELECTION_CONTENT:
		સંદેશ_data.selection.selection = true;
		તોડો;
	મૂળભૂત:
		તોડો;
	}
	સંદેશ_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((સંરચના content *)html, CONTENT_સંદેશ_SELECTION,
			&સંદેશ_data);
}
