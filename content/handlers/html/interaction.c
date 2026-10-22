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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/layout.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/keypress.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"
#ଅନ୍ତର୍ଭାଵ "desktop/browser_history.h"
#ଅନ୍ତର୍ଭାଵ "desktop/frames.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"
#ଅନ୍ତର୍ଭାଵ "javascript/js.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_textarea.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/imagemap.h"
#ଅନ୍ତର୍ଭାଵ "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ସ୍ଥିର browser_pointer_shape get_pointer_shape(ସଂରଚନା box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	ଗଣନା css_cursor_e cursor;
	lwc_string **cursor_uris;

	ଯଦି (box->type == BOX_FLOAT_ବାମ || box->type == BOX_FLOAT_ଡାହାଣ)
		style = box->children->style;
	ଅନ୍ୟଥା
		style = box->style;

	ଯଦି (style == ଖାଲି)
		ଫେରନ୍ତୁ BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	ବାଛ (cursor) {
	ସ୍ଥିତି CSS_CURSOR_AUTO:
		ଯଦି (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} ଅନ୍ୟଥା ଯଦି (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} ଅନ୍ୟଥା {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_ଡାହାଣ;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_ବାମ;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		pointer = BROWSER_POINTER_DEFAULT;
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ସ୍ଥିର ଶୂନ୍ୟ html_box_drag_start(ସଂରଚନା box *box, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x, box_y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ଯଦି (box->scroll_x != ଖାଲି) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ଶୀର୍ଷ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} ଅନ୍ୟଥା ଯଦି (box->scroll_y != ଖାଲି) {
		scroll_mouse_x = x - (box_x + box->padding[ବାମ_2] +
				box->width + box->padding[ଡାହାଣ_2] -
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
ସ୍ଥିର size_t html_selection_drag_end(ସଂରଚନା html_content *html,
		browser_mouse_state mouse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pixel_offset;
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ଯଦି (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	ଫେରନ୍ତୁ idx;
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
ସ୍ଥିର ଶୂନ୍ୟ
html__image_coords_dom_user_data_handler(dom_ଗଣ୍ଠି_operation operation,
					 dom_string *ଚାବି,
					 ଶୂନ୍ୟ *_data,
					 ସଂରଚନା dom_ଗଣ୍ଠି *src,
					 ସଂରଚନା dom_ଗଣ୍ଠି *dst)
{
	ସଂରଚନା image_ନିବେଶ_coords *oldcoords, *coords = _data, *newcoords;

	ଯଦି (!dom_string_isequal(corestring_dom_ns_ଚାବି_image_coords_ଗଣ୍ଠି_data,
				ଚାବି) || coords == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ବାଛ (operation) {
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_CLONED:
		newcoords = calloc(1, ଆକାର(*newcoords));
		ଯଦି (newcoords != ଖାଲି) {
			*newcoords = *coords;
			ଯଦି (dom_ଗଣ୍ଠି_set_user_data(dst,
				 corestring_dom_ns_ଚାବି_image_coords_ଗଣ୍ଠି_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				ମୁକ୍ତ(oldcoords);
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_DELETED:
		ମୁକ୍ତ(coords);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_RENAMED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_IMPORTED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_ADOPTED:
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ପ୍ରତିଜ୍ଞା(0);
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
ସ୍ଥିର ଶୂନ୍ୟ
html_overflow_scroll_drag_end(ସଂରଚନା scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ସଂରଚନା html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	ସଂରଚନା box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	ଯଦି (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[ଶୀର୍ଷ_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} ଅନ୍ୟଥା {
		scroll_mouse_x = x - (box_x + box->padding[ବାମ_2] +
				box->width + box->padding[ଡାହାଣ_2] -
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
ସ୍ଥିର nserror
mouse_action_select_menu(html_content *html,
			 ସଂରଚନା browser_window *bw,
			 browser_mouse_state mouse,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y = 0;
	ଅଚଳ ଅକ୍ଷର *status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
	ସଂରଚନା hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	ପ୍ରତିଜ୍ଞା(html->visible_select_menu != ଖାଲି);

	bw_drag_type = browser_window_get_drag_type(bw);
	ଯଦି (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[ବାମ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[ଶୀର୍ଷ_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ଯଦି (status != ଖାଲି) {
		/* set status if menu still open */
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
		ସନ୍ଦେଶ_data.explicit_status_text = status;
		content_broadcast((ସଂରଚନା content *)html,
				  CONTENT_ସନ୍ଦେଶ_STATUS,
				  &ସନ୍ଦେଶ_data);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = ଖାଲି;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ସ୍ଥିର nserror
mouse_action_drag_selection(html_content *html,
			    ସଂରଚନା browser_window *bw,
			    browser_mouse_state mouse,
			    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir = -1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dx, dy;
	size_t idx;
	ସଂଘ html_drag_owner drag_owner;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pixel_offset;
	plot_font_style_t fstyle;

	ଯଦି (!mouse) {
		/* End of selection drag */
		ଯଦି (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		ଯଦି (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, ଖାଲି);

		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ଯଦି (box != ଖାଲି) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ସ୍ଥିର nserror
mouse_action_drag_scrollbar(html_content *html,
			    ସଂରଚନା browser_window *bw,
			    browser_mouse_state mouse,
			    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା scrollbar *scr;
	ସଂରଚନା html_scrollbar_data *data;
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y = 0;
	ଅଚଳ ଅକ୍ଷର *status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	ଯଦି (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	ଯଦି (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[ଶୀର୍ଷ_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} ଅନ୍ୟଥା {
		scroll_mouse_x = x - (box_x + box->padding[ବାମ_2] +
				      box->width + box->padding[ଡାହାଣ_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	ଯଦି (status != ଖାଲି) {
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

		ସନ୍ଦେଶ_data.explicit_status_text = status;
		content_broadcast((ସଂରଚନା content *)html,
				  CONTENT_ସନ୍ଦେଶ_STATUS,
				  &ସନ୍ଦେଶ_data);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ସ୍ଥିର nserror
mouse_action_drag_textarea(html_content *html,
			    ସଂରଚନା browser_window *bw,
			    browser_mouse_state mouse,
			    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y = 0;

	box = html->drag_owner.textarea;

	ପ୍ରତିଜ୍ଞା(box->gadget != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ସ୍ଥିର nserror
mouse_action_drag_content(html_content *html,
			  ସଂରଚନା browser_window *bw,
			  browser_mouse_state mouse,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y = 0;

	box = html->drag_owner.content;
	ପ୍ରତିଜ୍ଞା(box->object != ଖାଲି);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ସଂରଚନା mouse_action_state {
	ସଂରଚନା {
		ଅଚଳ ଅକ୍ଷର *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		ଗଣନା {
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
	ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି;

	/** html object */
	ସଂରଚନା {
		ସଂରଚନା box *box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos_x;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ସଂରଚନା browser_window *iframe;

	/** link either from href or imagemap */
	ସଂରଚନା {
		ସଂରଚନା box *box;
		nsurl *url;
		ଅଚଳ ଅକ୍ଷର *target;
		bool is_imagemap;
	} link;

	/** gadget */
	ସଂରଚନା {
		ସଂରଚନା form_control *control;
		ସଂରଚନା box *box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y;
		ଅଚଳ ଅକ୍ଷର *target;
	} gadget;

	/** title */
	ଅଚଳ ଅକ୍ଷର *title;

	/** candidate box for drag operation */
	ସଂରଚନା box *drag_candidate;

	/** scrollbar */
	ସଂରଚନା {
		ସଂରଚନା scrollbar *bar;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା mouse_x;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା mouse_y;
	} scroll;

	/** text in box */
	ସଂରଚନା {
		ସଂରଚନା box *box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x;
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
ସ୍ଥିର nserror
get_mouse_action_ଗଣ୍ଠି(html_content *html,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		      ସଂରଚନା mouse_action_state *man)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, ଆକାର(ସଂରଚନା mouse_action_state));
	man->ଗଣ୍ଠି = html->layout->ଗଣ୍ଠି; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[ବାମ_2];
	box_y = box->margin[ଶୀର୍ଷ_2];

	କର {
		/* skip hidden boxes */
		ଯଦି ((box->style != ଖାଲି) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			ଯାଅ next_box;
		}

		ଯଦି (box->ଗଣ୍ଠି != ଖାଲି) {
			man->ଗଣ୍ଠି = box->ଗଣ୍ଠି;
		}

		ଯଦି (box->object) {
			ଯଦି (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} ଅନ୍ୟଥା {
				man->object = box->object;
			}
		}

		ଯଦି (box->iframe) {
			man->iframe = box->iframe;
		}

		ଯଦି (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ଯଦି (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ଯଦି (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ଯଦି (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		ଯଦି (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		ଯଦି ((box->scroll_x != ଖାଲି) ||
		    (box->scroll_y != ଖାଲି)) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding_ବାମ;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding_ଡାହାଣ;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding_ଶୀର୍ଷ;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding_bottom;

			ଯଦି (man->drag_candidate == ଖାଲି) {
				man->drag_candidate = box;
			}

			padding_ବାମ = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_ଡାହାଣ = padding_ବାମ + box->padding[ବାମ_2] +
					box->width + box->padding[ଡାହାଣ_2];
			padding_ଶୀର୍ଷ = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_ଶୀର୍ଷ + box->padding[ଶୀର୍ଷ_2] +
					box->height + box->padding[BOTTOM];

			ଯଦି ((x > padding_ବାମ) &&
			    (x < padding_ଡାହାଣ) &&
			    (y > padding_ଶୀର୍ଷ) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ଯଦି ((box->scroll_y != ଖାଲି) &&
				    (x > (padding_ଡାହାଣ - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_ଡାହାଣ - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_ଶୀର୍ଷ;
					ଭାଙ୍ଗ;

				} ଅନ୍ୟଥା ଯଦି ((box->scroll_x != ଖାଲି) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_ବାମ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					ଭାଙ୍ଗ;
				}
			}
		}

		ଯଦି (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ଯେତେବେଳେ (box != ଖାଲି);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	ପ୍ରତିଜ୍ଞା(man->ଗଣ୍ଠି != ଖାଲି);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ସ୍ଥିର nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		    ସଂରଚନା mouse_action_state *mas)
{
	ସଂରଚନା content *c = (ସଂରଚନା content *)html;
	textarea_mouse_status ta_status;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ବାଛ (mas->gadget.control->type) {
	ସ୍ଥିତି GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		ଯଦି (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ଯଦି (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = ଖାଲି;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
			ସନ୍ଦେଶ_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_ସନ୍ଦେଶ_SELECTMENU,
					  &ସନ୍ଦେଶ_data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_ନିବେଶ_element_set_checked(
				(dom_html_ନିବେଶ_element *)(mas->gadget.control->ଗଣ୍ଠି),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
			ସଂରଚନା image_ନିବେଶ_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, ଆକାର(*coords));
			ଯଦି (coords == ଖାଲି) {
				ଫେରନ୍ତୁ NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ଯଦି (dom_ଗଣ୍ଠି_set_user_data(
				mas->gadget.control->ଗଣ୍ଠି,
				corestring_dom_ns_ଚାବି_image_coords_ଗଣ୍ଠି_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				ଫେରନ୍ତୁ NSERROR_OK;
			}
			ମୁକ୍ତ(oldcoords);
		}
		fallthrough;

	ସ୍ଥିତି GADGET_SUBMIT:
		ଯଦି (mas->gadget.control->form) {
			ସ୍ଥିର ଅକ୍ଷର status_buffer[200];

			snprintf(status_buffer,
				 ଆକାର status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			ଯଦି (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} ଅନ୍ୟଥା {
			mas->result.status = messages_get("FormBadSubmit");
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_TEXTBOX:
	ସ୍ଥିତି GADGET_PASSWORD:
	ସ୍ଥିତି GADGET_TEXTAREA:
		ଯଦି (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} ଅନ୍ୟଥା {
			mas->result.status = messages_get("FormTextbox");
		}

		ଯଦି (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			ସଂଘ html_selection_owner sel_owner;
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

		ଯଦି (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} ଅନ୍ୟଥା {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_HIDDEN:
		/* not possible: no box generated */
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
			ସନ୍ଦେଶ_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_ସନ୍ଦେଶ_GADGETCLICK,
					  &ସନ୍ଦେଶ_data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ସ୍ଥିର nserror
iframe_mouse_action(ସଂରଚନା browser_window *bw,
		    browser_mouse_state mouse,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		    ସଂରଚନା mouse_action_state *mas)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos_x, pos_y;
	ଦଶମିକ scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	ଯଦି (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} ଅନ୍ୟଥା {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ସ୍ଥିର nserror
html_object_mouse_action(html_content *html,
			 ସଂରଚନା browser_window *bw,
			 browser_mouse_state mouse,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			 ସଂରଚନା mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ଯଦି (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		ସଂଘ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ଯଦି (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} ଅନ୍ୟଥା {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ସ୍ଥିର bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (scheme != ଖାଲି) {
		ଯଦି (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ଫେରନ୍ତୁ is_js;
}


/**
 * process mouse activity on a link
 */
ସ୍ଥିର nserror
link_mouse_action(html_content *html,
		  ସଂରଚନା browser_window *bw,
		  browser_mouse_state mouse,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		  ସଂରଚନା mouse_action_state *mas)
{
	nserror res;
	ଅକ୍ଷର *url_s = ଖାଲି;
	size_t url_l = 0;
	ସ୍ଥିର ଅକ୍ଷର status_buffer[200];
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

	ଯଦି (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ଯଦି (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = ଖାଲି;
		}
	}

	ଯଦି (mas->title) {
		snprintf(status_buffer,
			 ଆକାର status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} ଅନ୍ୟଥା {
		snprintf(status_buffer,
			 ଆକାର status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ଯଦି (url_s != ଖାଲି) {
		ମୁକ୍ତ(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	ଯଦି (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((ସଂରଚନା content *)html),
					BW_NAVIGATE_DOWNLOAD,
					ଖାଲି,
					ଖାଲି,
					ଖାଲି);

	} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		ସନ୍ଦେଶ_data.savelink.url = mas->link.url;
		ସନ୍ଦେଶ_data.savelink.title = mas->title;
		content_broadcast((ସଂରଚନା content *)html,
				  CONTENT_ସନ୍ଦେଶ_SAVELINK,
				  &ସନ୍ଦେଶ_data);

	} ଅନ୍ୟଥା ଯଦି (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ଯଦି (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} ଅନ୍ୟଥା {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ଯଦି (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		ସଂଘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ଖାଲି);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ସ୍ଥିର nserror
default_mouse_action(html_content *html,
		  ସଂରଚନା browser_window *bw,
		  browser_mouse_state mouse,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		  ସଂରଚନା mouse_action_state *mas)
{
	ସଂରଚନା content *c = (ସଂରଚନା content *)html;

	/* frame resizing */
	ଯଦି (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		ଯଦି (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		ଫେରନ୍ତୁ default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	ସଂଘ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ଯଦି (click && html->focus_type != HTML_FOCUS_SELF) {
		ସଂଘ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, ଖାଲି);
	}
	ଯଦି (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ଯଦି (mas->text.box) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା pixel_offset;
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

		ଯଦି (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			ସଂଘ html_drag_owner drag_owner;

			ଯଦି (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   ଖାଲି);
				mas->result.status = messages_get("Selecting");
			}

			ଯଦି (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} ଅନ୍ୟଥା ଯଦି (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ଫେରନ୍ତୁ default_mouse_action_focus(html, mouse);
		}

	} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ଯଦି (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} ଅନ୍ୟଥା ଯଦି (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ଯଦି (mas->title) {
		mas->result.status = mas->title;
	}

	ଯଦି (mouse & BROWSER_MOUSE_DRAG_1) {
		ଯଦି (mouse & BROWSER_MOUSE_MOD_2) {
			ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
			ସନ୍ଦେଶ_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			ସନ୍ଦେଶ_data.dragsave.content = ଖାଲି;
			content_broadcast(c, CONTENT_ସନ୍ଦେଶ_DRAGSAVE, &ସନ୍ଦେଶ_data);
		} ଅନ୍ୟଥା {
			ଯଦି (mas->drag_candidate == ଖାଲି) {
				browser_window_page_drag_start(bw, x, y);
			} ଅନ୍ୟଥା {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_DRAG_2) {
		ଯଦି (mouse & BROWSER_MOUSE_MOD_2) {
			ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
			ସନ୍ଦେଶ_data.dragsave.type = CONTENT_SAVE_SOURCE;
			ସନ୍ଦେଶ_data.dragsave.content = ଖାଲି;
			content_broadcast(c, CONTENT_ସନ୍ଦେଶ_DRAGSAVE, &ସନ୍ଦେଶ_data);
		} ଅନ୍ୟଥା {
			ଯଦି (mas->drag_candidate == ଖାଲି) {
				browser_window_page_drag_start(bw, x, y);
			} ଅନ୍ୟଥା {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	ଫେରନ୍ତୁ default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ସ୍ଥିର nserror
mouse_action_drag_none(html_content *html,
		       ସଂରଚନା browser_window *bw,
		       browser_mouse_state mouse,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	nserror res;
	ସଂରଚନା content *c = (ସଂରଚନା content *)html;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ସ୍ଥିର ସଂରଚନା mouse_action_state mas;

	res = get_mouse_action_ଗଣ୍ଠି(html, x, y, &mas);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ଯଦି (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} ଅନ୍ୟଥା ଯଦି (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} ଅନ୍ୟଥା ଯଦି (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} ଅନ୍ୟଥା ଯଦି ((mas.object != ଖାଲି) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ଯଦି (mouse & BROWSER_MOUSE_DRAG_2) {
			ସନ୍ଦେଶ_data.dragsave.type = CONTENT_SAVE_NATIVE;
			ସନ୍ଦେଶ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_ସନ୍ଦେଶ_DRAGSAVE, &ସନ୍ଦେଶ_data);

		} ଅନ୍ୟଥା ଯଦି (mouse & BROWSER_MOUSE_DRAG_1) {
			ସନ୍ଦେଶ_data.dragsave.type = CONTENT_SAVE_ORIG;
			ସନ୍ଦେଶ_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_ସନ୍ଦେଶ_DRAGSAVE, &ସନ୍ଦେଶ_data);
		}

		/* \todo should have a drag-saving object msg */

	} ଅନ୍ୟଥା ଯଦି (mas.iframe != ଖାଲି) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} ଅନ୍ୟଥା ଯଦି (mas.html_object.box != ଖାଲି) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} ଅନ୍ୟଥା ଯଦି (mas.link.url != ଖାଲି) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} ଅନ୍ୟଥା {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	/* send status and pointer message */
	ଯଦି (mas.result.action != ACTION_NOSEND) {
		ସନ୍ଦେଶ_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_STATUS, &ସନ୍ଦେଶ_data);

		ସନ୍ଦେଶ_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_POINTER, &ସନ୍ଦେଶ_data);
	}

	/* fire dom click event */
	ଯଦି (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ଗଣ୍ଠି, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	ବାଛ (mas.result.action) {
	ସ୍ଥିତି ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				ଖାଲି,
				ଖାଲି,
				ଖାଲି);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		ଯଦି (path != ଖାଲି) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି ACTION_BACK:
		res = browser_window_history_back(bw, false);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି ACTION_NOSEND:
	ସ୍ଥିତି ACTION_NONE:
		res = NSERROR_OK;
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ସଂରଚନା content *c,
			 ସଂରଚନା browser_window *bw,
			 browser_mouse_state mouse,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ଫେରନ୍ତୁ html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ସଂରଚନା content *c,
		  ସଂରଚନା browser_window *bw,
		  browser_mouse_state mouse,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ଯଦି (html->visible_select_menu != ଖାଲି) {
		ଫେରନ୍ତୁ mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	ବାଛ (html->drag_type) {
	ସ୍ଥିତି HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_TEXTAREA_SELECTION:
	ସ୍ଥିତି HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_CONTENT_SELECTION:
	ସ୍ଥିତି HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Unknown content related drag type */
		ପ୍ରତିଜ୍ଞା(0 && "Unknown content related drag type");
	}

	ଯଦି (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ଫେରନ୍ତୁ res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ସଂରଚନା content *c, uint32_t ଚାବି)
{
	html_content *html = (html_content *) c;
	ସଂରଚନା selection *sel = html->sel;

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
	ଯଦି (html->layout != ଖାଲି && html->layout->ଗଣ୍ଠି != ଖାଲି) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ଗଣ୍ଠି, true, true, ଚାବି);
	}

	ବାଛ (html->focus_type) {
	ସ୍ଥିତି HTML_FOCUS_CONTENT:
		ଫେରନ୍ତୁ content_keypress(html->focus_owner.content->object, ଚାବି);

	ସ୍ଥିତି HTML_FOCUS_TEXTAREA:
		ଯଦି (box_textarea_keypress(html, html->focus_owner.textarea, ଚାବି) == NSERROR_OK) {
			ଫେରନ୍ତୁ true;
		} ଅନ୍ୟଥା {
			ଫେରନ୍ତୁ false;
		}

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Deal with it below */
		ଭାଙ୍ଗ;
	}

	ବାଛ (ଚାବି) {
	ସ୍ଥିତି NS_ଚାବି_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_CLEAR_SELECTION:
		selection_clear(sel, true);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_SELECT_ALL:
		selection_select_all(sel);
		ଫେରନ୍ତୁ true;

	ସ୍ଥିତି NS_ଚାବି_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ଫେରନ୍ତୁ selection_clear(sel, true);
	}

	ଫେରନ୍ତୁ false;
}


/**
 * Callback for in-page scrollbars.
 */
ଶୂନ୍ୟ html_overflow_scroll_callback(ଶୂନ୍ୟ *client_data,
		ସଂରଚନା scrollbar_ସନ୍ଦେଶ_data *scrollbar_data)
{
	ସଂରଚନା html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	ସଂରଚନା box *box = data->box;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	html_drag_type drag_type;
	ସଂଘ html_drag_owner drag_owner;

	ବାଛ(scrollbar_data->ସନ୍ଦେଶ) {
	ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_MOVED:

		ଯଦି (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ଭାଙ୍ଗ;
		}

		html__redraw_a_box(html, box);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_SCROLL_START:
	{
		ସଂରଚନା rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, ଖାଲି);

		ସନ୍ଦେଶ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_ସନ୍ଦେଶ_POINTER, &ସନ୍ଦେଶ_data);
		ଭାଙ୍ଗ;
	}
}


/* Documented in html_internal.h */
ଶୂନ୍ୟ html_set_drag_type(html_content *html, html_drag_type drag_type,
		ସଂଘ html_drag_owner drag_owner, ଅଚଳ ସଂରଚନା rect *rect)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

	ପ୍ରତିଜ୍ଞା(html != ଖାଲି);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	ବାଛ (drag_type) {
	ସ୍ଥିତି HTML_DRAG_NONE:
		ପ୍ରତିଜ୍ଞା(drag_owner.no_owner == true);
		ସନ୍ଦେଶ_data.drag.type = CONTENT_DRAG_NONE;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_SCROLLBAR:
	ସ୍ଥିତି HTML_DRAG_TEXTAREA_SCROLLBAR:
	ସ୍ଥିତି HTML_DRAG_CONTENT_SCROLL:
		ସନ୍ଦେଶ_data.drag.type = CONTENT_DRAG_SCROLL;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_DRAG_SELECTION:
		ପ୍ରତିଜ୍ଞା(drag_owner.no_owner == true);
		fallthrough;
	ସ୍ଥିତି HTML_DRAG_TEXTAREA_SELECTION:
	ସ୍ଥିତି HTML_DRAG_CONTENT_SELECTION:
		ସନ୍ଦେଶ_data.drag.type = CONTENT_DRAG_SELECTION;
		ଭାଙ୍ଗ;
	}
	ସନ୍ଦେଶ_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((ସଂରଚନା content *)html, CONTENT_ସନ୍ଦେଶ_DRAG, &ସନ୍ଦେଶ_data);
}

/* Documented in html_internal.h */
ଶୂନ୍ୟ html_set_focus(html_content *html, html_focus_type focus_type,
		ସଂଘ html_focus_owner focus_owner, bool hide_caret,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ଅଚଳ ସଂରଚନା rect *clip)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_off = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_off = 0;
	ସଂରଚନା rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	ପ୍ରତିଜ୍ଞା(html != ଖାଲି);

	ବାଛ (focus_type) {
	ସ୍ଥିତି HTML_FOCUS_SELF:
		ପ୍ରତିଜ୍ଞା(focus_owner.self == true);
		ଯଦି (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ଫେରନ୍ତୁ;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		ଭାଙ୍ଗ;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ଯଦି (textarea_lost_focus) {
		ସନ୍ଦେଶ_data.caret.type = CONTENT_CARET_REMOVE;
	} ଅନ୍ୟଥା ଯଦି (focus_type != HTML_FOCUS_SELF && hide_caret) {
		ସନ୍ଦେଶ_data.caret.type = CONTENT_CARET_HIDE;
	} ଅନ୍ୟଥା {
		ଯଦି (clip != ଖାଲି) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		ସନ୍ଦେଶ_data.caret.type = CONTENT_CARET_SET_POS;
		ସନ୍ଦେଶ_data.caret.pos.x = x + x_off;
		ସନ୍ଦେଶ_data.caret.pos.y = y + y_off;
		ସନ୍ଦେଶ_data.caret.pos.height = height;
		ସନ୍ଦେଶ_data.caret.pos.clip = (clip == ଖାଲି) ? ଖାଲି : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((ସଂରଚନା content *)html, CONTENT_ସନ୍ଦେଶ_CARET, &ସନ୍ଦେଶ_data);
}

/* Documented in html_internal.h */
ଶୂନ୍ୟ html_set_selection(html_content *html, html_selection_type selection_type,
		ସଂଘ html_selection_owner selection_owner, bool read_only)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ସଂରଚନା box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	ପ୍ରତିଜ୍ଞା(html != ଖାଲି);

	ଯଦି ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	ଯଦି (html->selection_type != HTML_SELECTION_NONE) {
		ବାଛ (html->selection_type) {
		ସ୍ଥିତି HTML_SELECTION_SELF:
			ଯଦି (same_type)
				ଭାଙ୍ଗ;
			selection_clear(html->sel, true);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି HTML_SELECTION_TEXTAREA:
			ଯଦି (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				ଭାଙ୍ଗ;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି HTML_SELECTION_CONTENT:
			ଯଦି (same_type && html->selection_owner.content ==
					selection_owner.content)
				ଭାଙ୍ଗ;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ଯଦି (!changed)
		/* Don't need to report lack of change to owner */
		ଫେରନ୍ତୁ;

	/* Prepare msg */
	ବାଛ (selection_type) {
	ସ୍ଥିତି HTML_SELECTION_NONE:
		ପ୍ରତିଜ୍ଞା(selection_owner.none == true);
		ସନ୍ଦେଶ_data.selection.selection = false;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HTML_SELECTION_SELF:
		ପ୍ରତିଜ୍ଞା(selection_owner.none == false);
		fallthrough;
	ସ୍ଥିତି HTML_SELECTION_TEXTAREA:
	ସ୍ଥିତି HTML_SELECTION_CONTENT:
		ସନ୍ଦେଶ_data.selection.selection = true;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}
	ସନ୍ଦେଶ_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((ସଂରଚନା content *)html, CONTENT_ସନ୍ଦେଶ_SELECTION,
			&ସନ୍ଦେଶ_data);
}
