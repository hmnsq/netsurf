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

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <dom/dom.h>

#include "utils/corestrings.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/log.h"
#include "utils/nsoption.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/mouse.h"
#include "netsurf/misc.h"
#include "netsurf/layout.h"
#include "netsurf/keypress.h"
#include "content/hlcache.h"
#include "content/textsearch.h"
#include "desktop/browser_history.h"
#include "desktop/frames.h"
#include "desktop/scrollbar.h"
#include "desktop/selection.h"
#include "desktop/textarea.h"
#include "javascript/js.h"
#include "desktop/gui_internal.h"

#include "html/box.h"
#include "html/box_textarea.h"
#include "html/box_inspect.h"
#include "html/font.h"
#include "html/form_internal.h"
#include "html/private.h"
#include "html/imagemap.h"
#include "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

static browser_pointer_shape get_pointer_shape(sa_nracanaa box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	ga_nanaa css_cursor_e cursor;
	lwc_string **cursor_uris;

	ถ้า (box->type == BOX_FLOAT_LEFT || box->type == BOX_FLOAT_RIGHT)
		style = box->children->style;
	มิฉะนั้น
		style = box->style;

	ถ้า (style == NULL)
		คืน BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	switch (cursor) {
	pak_sa CSS_CURSOR_AUTO:
		ถ้า (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} มิฉะนั้น ถ้า (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} มิฉะนั้น {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		bha_nga;
	pak_sa CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		bha_nga;
	pak_sa CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		bha_nga;
	pak_sa CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		bha_nga;
	pak_sa CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_RIGHT;
		bha_nga;
	pak_sa CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_LEFT;
		bha_nga;
	pak_sa CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		bha_nga;
	pak_sa CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		bha_nga;
	pak_sa CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		bha_nga;
	pak_sa CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		bha_nga;
	pak_sa CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		bha_nga;
	pak_sa CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		bha_nga;
	pak_sa CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		bha_nga;
	pak_sa CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		bha_nga;
	pak_sa CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		bha_nga;
	pak_sa CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		bha_nga;
	default:
		pointer = BROWSER_POINTER_DEFAULT;
		bha_nga;
	}

	คืน pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

static void html_box_drag_start(sa_nracanaa box *box, จำนวนเต็ม x, จำนวนเต็ม y)
{
	จำนวนเต็ม box_x, box_y;
	จำนวนเต็ม scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	ถ้า (box->scroll_x != NULL) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} มิฉะนั้น ถ้า (box->scroll_y != NULL) {
		scroll_mouse_x = x - (box_x + box->padding[LEFT] +
				box->width + box->padding[RIGHT] -
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
static size_t html_selection_drag_end(sa_nracanaa html_content *html,
		browser_mouse_state mouse, จำนวนเต็ม x, จำนวนเต็ม y, จำนวนเต็ม dir)
{
	จำนวนเต็ม pixel_offset;
	sa_nracanaa box *box;
	จำนวนเต็ม dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ถ้า (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	คืน idx;
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
static void
html__image_coords_dom_user_data_handler(dom_node_operation operation,
					 dom_string *key,
					 void *_data,
					 sa_nracanaa dom_node *src,
					 sa_nracanaa dom_node *dst)
{
	sa_nracanaa image_input_coords *oldcoords, *coords = _data, *newcoords;

	ถ้า (!dom_string_isequal(corestring_dom___ns_key_image_coords_node_data,
				key) || coords == NULL) {
		คืน;
	}

	switch (operation) {
	pak_sa DOM_NODE_CLONED:
		newcoords = calloc(1, parimaa_na(*newcoords));
		ถ้า (newcoords != NULL) {
			*newcoords = *coords;
			ถ้า (dom_node_set_user_data(dst,
				 corestring_dom___ns_key_image_coords_node_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				free(oldcoords);
			}
		}
		bha_nga;

	pak_sa DOM_NODE_DELETED:
		free(coords);
		bha_nga;

	pak_sa DOM_NODE_RENAMED:
	pak_sa DOM_NODE_IMPORTED:
	pak_sa DOM_NODE_ADOPTED:
		bha_nga;

	default:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		assert(0);
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
static void
html_overflow_scroll_drag_end(sa_nracanaa scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      จำนวนเต็ม x, จำนวนเต็ม y)
{
	จำนวนเต็ม scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	sa_nracanaa html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	sa_nracanaa box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	ถ้า (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} มิฉะนั้น {
		scroll_mouse_x = x - (box_x + box->padding[LEFT] +
				box->width + box->padding[RIGHT] -
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
static nserror
mouse_action_select_menu(html_content *html,
			 sa_nracanaa browser_window *bw,
			 browser_mouse_state mouse,
			 จำนวนเต็ม x, จำนวนเต็ม y)
{
	sa_nracanaa box *box;
	จำนวนเต็ม box_x = 0;
	จำนวนเต็ม box_y = 0;
	const var_na *status;
	จำนวนเต็ม width, height;
	sa_nracanaa hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	assert(html->visible_select_menu != NULL);

	bw_drag_type = browser_window_get_drag_type(bw);
	ถ้า (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[LEFT].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[TOP];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	ถ้า (status != NULL) {
		/* set status if menu still open */
		sa_ngha content_msg_data msg_data;
		msg_data.explicit_status_text = status;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_STATUS,
				  &msg_data);
		คืน NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = NULL;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	คืน NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
static nserror
mouse_action_drag_selection(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    จำนวนเต็ม x, จำนวนเต็ม y)
{
	sa_nracanaa box *box;
	จำนวนเต็ม dir = -1;
	จำนวนเต็ม dx, dy;
	size_t idx;
	sa_ngha html_drag_owner drag_owner;
	จำนวนเต็ม pixel_offset;
	plot_font_style_t fstyle;

	ถ้า (!mouse) {
		/* End of selection drag */
		ถ้า (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		ถ้า (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, NULL);

		คืน NSERROR_OK;
	}

	ถ้า (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	ถ้า (box != NULL) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	คืน NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
static nserror
mouse_action_drag_scrollbar(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    จำนวนเต็ม x, จำนวนเต็ม y)
{
	sa_nracanaa scrollbar *scr;
	sa_nracanaa html_scrollbar_data *data;
	sa_nracanaa box *box;
	จำนวนเต็ม box_x = 0;
	จำนวนเต็ม box_y = 0;
	const var_na *status;
	จำนวนเต็ม scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	ถ้า (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	ถ้า (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} มิฉะนั้น {
		scroll_mouse_x = x - (box_x + box->padding[LEFT] +
				      box->width + box->padding[RIGHT] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	ถ้า (status != NULL) {
		sa_ngha content_msg_data msg_data;

		msg_data.explicit_status_text = status;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_STATUS,
				  &msg_data);
	}

	คืน NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
static nserror
mouse_action_drag_textarea(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    จำนวนเต็ม x, จำนวนเต็ม y)
{
	sa_nracanaa box *box;
	จำนวนเต็ม box_x = 0;
	จำนวนเต็ม box_y = 0;

	box = html->drag_owner.textarea;

	assert(box->gadget != NULL);
	assert(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	คืน NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
static nserror
mouse_action_drag_content(html_content *html,
			  sa_nracanaa browser_window *bw,
			  browser_mouse_state mouse,
			  จำนวนเต็ม x, จำนวนเต็ม y)
{
	sa_nracanaa box *box;
	จำนวนเต็ม box_x = 0;
	จำนวนเต็ม box_y = 0;

	box = html->drag_owner.content;
	assert(box->object != NULL);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	คืน NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
sa_nracanaa mouse_action_state {
	sa_nracanaa {
		const var_na *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		ga_nanaa {
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
	sa_nracanaa dom_node *node;

	/** html object */
	sa_nracanaa {
		sa_nracanaa box *box;
		จำนวนเต็ม pos_x;
		จำนวนเต็ม pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	sa_nracanaa browser_window *iframe;

	/** link either from href or imagemap */
	sa_nracanaa {
		sa_nracanaa box *box;
		nsurl *url;
		const var_na *target;
		bool is_imagemap;
	} link;

	/** gadget */
	sa_nracanaa {
		sa_nracanaa form_control *control;
		sa_nracanaa box *box;
		จำนวนเต็ม box_x;
		จำนวนเต็ม box_y;
		const var_na *target;
	} gadget;

	/** title */
	const var_na *title;

	/** candidate box for drag operation */
	sa_nracanaa box *drag_candidate;

	/** scrollbar */
	sa_nracanaa {
		sa_nracanaa scrollbar *bar;
		จำนวนเต็ม mouse_x;
		จำนวนเต็ม mouse_y;
	} scroll;

	/** text in box */
	sa_nracanaa {
		sa_nracanaa box *box;
		จำนวนเต็ม box_x;
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
static nserror
get_mouse_action_node(html_content *html,
		      จำนวนเต็ม x, จำนวนเต็ม y,
		      sa_nracanaa mouse_action_state *man)
{
	sa_nracanaa box *box;
	จำนวนเต็ม box_x = 0;
	จำนวนเต็ม box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, parimaa_na(sa_nracanaa mouse_action_state));
	man->node = html->layout->node; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[LEFT];
	box_y = box->margin[TOP];

	do {
		/* skip hidden boxes */
		ถ้า ((box->style != NULL) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			goto next_box;
		}

		ถ้า (box->node != NULL) {
			man->node = box->node;
		}

		ถ้า (box->object) {
			ถ้า (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} มิฉะนั้น {
				man->object = box->object;
			}
		}

		ถ้า (box->iframe) {
			man->iframe = box->iframe;
		}

		ถ้า (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		ถ้า (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		ถ้า (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			ถ้า (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		ถ้า (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		ถ้า ((box->scroll_x != NULL) ||
		    (box->scroll_y != NULL)) {
			จำนวนเต็ม padding_left;
			จำนวนเต็ม padding_right;
			จำนวนเต็ม padding_top;
			จำนวนเต็ม padding_bottom;

			ถ้า (man->drag_candidate == NULL) {
				man->drag_candidate = box;
			}

			padding_left = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_right = padding_left + box->padding[LEFT] +
					box->width + box->padding[RIGHT];
			padding_top = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_top + box->padding[TOP] +
					box->height + box->padding[BOTTOM];

			ถ้า ((x > padding_left) &&
			    (x < padding_right) &&
			    (y > padding_top) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				ถ้า ((box->scroll_y != NULL) &&
				    (x > (padding_right - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_right - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_top;
					bha_nga;

				} มิฉะนั้น ถ้า ((box->scroll_x != NULL) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_left;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					bha_nga;
				}
			}
		}

		ถ้า (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} ขณะ (box != NULL);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	assert(man->node != NULL);

	คืน NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
static nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    จำนวนเต็ม x, จำนวนเต็ม y,
		    sa_nracanaa mouse_action_state *mas)
{
	sa_nracanaa content *c = (sa_nracanaa content *)html;
	textarea_mouse_status ta_status;
	sa_ngha content_msg_data msg_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	switch (mas->gadget.control->type) {
	pak_sa GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		ถ้า (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			ถ้า (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = NULL;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
			msg_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_MSG_SELECTMENU,
					  &msg_data);
		}
		bha_nga;

	pak_sa GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_input_element_set_checked(
				(dom_html_input_element *)(mas->gadget.control->node),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		bha_nga;

	pak_sa GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		bha_nga;

	pak_sa GADGET_IMAGE:
		/* This falls through to SUBMIT */
		ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
			sa_nracanaa image_input_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, parimaa_na(*coords));
			ถ้า (coords == NULL) {
				คืน NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			ถ้า (dom_node_set_user_data(
				mas->gadget.control->node,
				corestring_dom___ns_key_image_coords_node_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				คืน NSERROR_OK;
			}
			free(oldcoords);
		}
		fallthrough;

	pak_sa GADGET_SUBMIT:
		ถ้า (mas->gadget.control->form) {
			static var_na status_buffer[200];

			snprintf(status_buffer,
				 parimaa_na status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			ถ้า (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} มิฉะนั้น {
			mas->result.status = messages_get("FormBadSubmit");
		}
		bha_nga;

	pak_sa GADGET_TEXTBOX:
	pak_sa GADGET_PASSWORD:
	pak_sa GADGET_TEXTAREA:
		ถ้า (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} มิฉะนั้น {
			mas->result.status = messages_get("FormTextbox");
		}

		ถ้า (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			sa_ngha html_selection_owner sel_owner;
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

		ถ้า (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} มิฉะนั้น {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		bha_nga;

	pak_sa GADGET_HIDDEN:
		/* not possible: no box generated */
		bha_nga;

	pak_sa GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		bha_nga;

	pak_sa GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
			msg_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_MSG_GADGETCLICK,
					  &msg_data);
		}
		bha_nga;

	pak_sa GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		bha_nga;
	}

	คืน NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
static nserror
iframe_mouse_action(sa_nracanaa browser_window *bw,
		    browser_mouse_state mouse,
		    จำนวนเต็ม x, จำนวนเต็ม y,
		    sa_nracanaa mouse_action_state *mas)
{
	จำนวนเต็ม pos_x, pos_y;
	float scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	ถ้า (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} มิฉะนั้น {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	คืน NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
static nserror
html_object_mouse_action(html_content *html,
			 sa_nracanaa browser_window *bw,
			 browser_mouse_state mouse,
			 จำนวนเต็ม x, จำนวนเต็ม y,
			 sa_nracanaa mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ถ้า (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		sa_ngha html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ถ้า (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} มิฉะนั้น {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	คืน NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
static bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ถ้า (scheme != NULL) {
		ถ้า (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	คืน is_js;
}


/**
 * process mouse activity on a link
 */
static nserror
link_mouse_action(html_content *html,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  จำนวนเต็ม x, จำนวนเต็ม y,
		  sa_nracanaa mouse_action_state *mas)
{
	nserror res;
	var_na *url_s = NULL;
	size_t url_l = 0;
	static var_na status_buffer[200];
	sa_ngha content_msg_data msg_data;

	ถ้า (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		ถ้า (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = NULL;
		}
	}

	ถ้า (mas->title) {
		snprintf(status_buffer,
			 parimaa_na status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} มิฉะนั้น {
		snprintf(status_buffer,
			 parimaa_na status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	ถ้า (url_s != NULL) {
		free(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	ถ้า (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((sa_nracanaa content *)html),
					BW_NAVIGATE_DOWNLOAD,
					NULL,
					NULL,
					NULL);

	} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		msg_data.savelink.url = mas->link.url;
		msg_data.savelink.title = mas->title;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_SAVELINK,
				  &msg_data);

	} มิฉะนั้น ถ้า (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		ถ้า (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} มิฉะนั้น {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	คืน NSERROR_OK;
}


static nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	ถ้า (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		sa_ngha html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, NULL);
	}

	คืน NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
static nserror
default_mouse_action(html_content *html,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  จำนวนเต็ม x, จำนวนเต็ม y,
		  sa_nracanaa mouse_action_state *mas)
{
	sa_nracanaa content *c = (sa_nracanaa content *)html;

	/* frame resizing */
	ถ้า (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		ถ้า (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		คืน default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	sa_ngha html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	ถ้า (click && html->focus_type != HTML_FOCUS_SELF) {
		sa_ngha html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, NULL);
	}
	ถ้า (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ถ้า (mas->text.box) {
		จำนวนเต็ม pixel_offset;
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

		ถ้า (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			sa_ngha html_drag_owner drag_owner;

			ถ้า (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   NULL);
				mas->result.status = messages_get("Selecting");
			}

			ถ้า (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} มิฉะนั้น ถ้า (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			คืน default_mouse_action_focus(html, mouse);
		}

	} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	ถ้า (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} มิฉะนั้น ถ้า (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	ถ้า (mas->title) {
		mas->result.status = mas->title;
	}

	ถ้า (mouse & BROWSER_MOUSE_DRAG_1) {
		ถ้า (mouse & BROWSER_MOUSE_MOD_2) {
			sa_ngha content_msg_data msg_data;
			msg_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			msg_data.dragsave.content = NULL;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		} มิฉะนั้น {
			ถ้า (mas->drag_candidate == NULL) {
				browser_window_page_drag_start(bw, x, y);
			} มิฉะนั้น {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_DRAG_2) {
		ถ้า (mouse & BROWSER_MOUSE_MOD_2) {
			sa_ngha content_msg_data msg_data;
			msg_data.dragsave.type = CONTENT_SAVE_SOURCE;
			msg_data.dragsave.content = NULL;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		} มิฉะนั้น {
			ถ้า (mas->drag_candidate == NULL) {
				browser_window_page_drag_start(bw, x, y);
			} มิฉะนั้น {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	คืน default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
static nserror
mouse_action_drag_none(html_content *html,
		       sa_nracanaa browser_window *bw,
		       browser_mouse_state mouse,
		       จำนวนเต็ม x, จำนวนเต็ม y)
{
	nserror res;
	sa_nracanaa content *c = (sa_nracanaa content *)html;
	sa_ngha content_msg_data msg_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	static sa_nracanaa mouse_action_state mas;

	res = get_mouse_action_node(html, x, y, &mas);
	ถ้า (res != NSERROR_OK) {
		คืน res;
	}

	ถ้า (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} มิฉะนั้น ถ้า (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} มิฉะนั้น ถ้า (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} มิฉะนั้น ถ้า ((mas.object != NULL) && (mouse & BROWSER_MOUSE_MOD_2)) {

		ถ้า (mouse & BROWSER_MOUSE_DRAG_2) {
			msg_data.dragsave.type = CONTENT_SAVE_NATIVE;
			msg_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);

		} มิฉะนั้น ถ้า (mouse & BROWSER_MOUSE_DRAG_1) {
			msg_data.dragsave.type = CONTENT_SAVE_ORIG;
			msg_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		}

		/* \todo should have a drag-saving object msg */

	} มิฉะนั้น ถ้า (mas.iframe != NULL) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} มิฉะนั้น ถ้า (mas.html_object.box != NULL) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} มิฉะนั้น ถ้า (mas.link.url != NULL) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} มิฉะนั้น {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	ถ้า (res != NSERROR_OK) {
		คืน res;
	}

	/* send status and pointer message */
	ถ้า (mas.result.action != ACTION_NOSEND) {
		msg_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

		msg_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);
	}

	/* fire dom click event */
	ถ้า (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.node, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	switch (mas.result.action) {
	pak_sa ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		bha_nga;

	pak_sa ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				NULL,
				NULL,
				NULL);
		bha_nga;

	pak_sa ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		ถ้า (path != NULL) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		bha_nga;

	pak_sa ACTION_BACK:
		res = browser_window_history_back(bw, false);
		bha_nga;

	pak_sa ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		bha_nga;

	pak_sa ACTION_NOSEND:
	pak_sa ACTION_NONE:
		res = NSERROR_OK;
		bha_nga;
	}

	คืน res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(sa_nracanaa content *c,
			 sa_nracanaa browser_window *bw,
			 browser_mouse_state mouse,
			 จำนวนเต็ม x, จำนวนเต็ม y)
{
	คืน html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(sa_nracanaa content *c,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  จำนวนเต็ม x, จำนวนเต็ม y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	ถ้า (html->visible_select_menu != NULL) {
		คืน mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	switch (html->drag_type) {
	pak_sa HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		bha_nga;

	pak_sa HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		bha_nga;

	pak_sa HTML_DRAG_TEXTAREA_SELECTION:
	pak_sa HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		bha_nga;

	pak_sa HTML_DRAG_CONTENT_SELECTION:
	pak_sa HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		bha_nga;

	pak_sa HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		bha_nga;

	default:
		/* Unknown content related drag type */
		assert(0 && "Unknown content related drag type");
	}

	ถ้า (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	คืน res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(sa_nracanaa content *c, uint32_t key)
{
	html_content *html = (html_content *) c;
	sa_nracanaa selection *sel = html->sel;

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
	ถ้า (html->layout != NULL && html->layout->node != NULL) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->node, true, true, key);
	}

	switch (html->focus_type) {
	pak_sa HTML_FOCUS_CONTENT:
		คืน content_keypress(html->focus_owner.content->object, key);

	pak_sa HTML_FOCUS_TEXTAREA:
		ถ้า (box_textarea_keypress(html, html->focus_owner.textarea, key) == NSERROR_OK) {
			คืน true;
		} มิฉะนั้น {
			คืน false;
		}

	default:
		/* Deal with it below */
		bha_nga;
	}

	switch (key) {
	pak_sa NS_KEY_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		คืน true;

	pak_sa NS_KEY_CLEAR_SELECTION:
		selection_clear(sel, true);
		คืน true;

	pak_sa NS_KEY_SELECT_ALL:
		selection_select_all(sel);
		คืน true;

	pak_sa NS_KEY_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		คืน selection_clear(sel, true);
	}

	คืน false;
}


/**
 * Callback for in-page scrollbars.
 */
void html_overflow_scroll_callback(void *client_data,
		sa_nracanaa scrollbar_msg_data *scrollbar_data)
{
	sa_nracanaa html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	sa_nracanaa box *box = data->box;
	sa_ngha content_msg_data msg_data;
	html_drag_type drag_type;
	sa_ngha html_drag_owner drag_owner;

	switch(scrollbar_data->msg) {
	pak_sa SCROLLBAR_MSG_MOVED:

		ถ้า (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			bha_nga;
		}

		html__redraw_a_box(html, box);
		bha_nga;
	pak_sa SCROLLBAR_MSG_SCROLL_START:
	{
		sa_nracanaa rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		bha_nga;
	pak_sa SCROLLBAR_MSG_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, NULL);

		msg_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_MSG_POINTER, &msg_data);
		bha_nga;
	}
}


/* Documented in html_internal.h */
void html_set_drag_type(html_content *html, html_drag_type drag_type,
		sa_ngha html_drag_owner drag_owner, const sa_nracanaa rect *rect)
{
	sa_ngha content_msg_data msg_data;

	assert(html != NULL);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	switch (drag_type) {
	pak_sa HTML_DRAG_NONE:
		assert(drag_owner.no_owner == true);
		msg_data.drag.type = CONTENT_DRAG_NONE;
		bha_nga;

	pak_sa HTML_DRAG_SCROLLBAR:
	pak_sa HTML_DRAG_TEXTAREA_SCROLLBAR:
	pak_sa HTML_DRAG_CONTENT_SCROLL:
		msg_data.drag.type = CONTENT_DRAG_SCROLL;
		bha_nga;

	pak_sa HTML_DRAG_SELECTION:
		assert(drag_owner.no_owner == true);
		fallthrough;
	pak_sa HTML_DRAG_TEXTAREA_SELECTION:
	pak_sa HTML_DRAG_CONTENT_SELECTION:
		msg_data.drag.type = CONTENT_DRAG_SELECTION;
		bha_nga;
	}
	msg_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((sa_nracanaa content *)html, CONTENT_MSG_DRAG, &msg_data);
}

/* Documented in html_internal.h */
void html_set_focus(html_content *html, html_focus_type focus_type,
		sa_ngha html_focus_owner focus_owner, bool hide_caret,
		จำนวนเต็ม x, จำนวนเต็ม y, จำนวนเต็ม height, const sa_nracanaa rect *clip)
{
	sa_ngha content_msg_data msg_data;
	จำนวนเต็ม x_off = 0;
	จำนวนเต็ม y_off = 0;
	sa_nracanaa rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	assert(html != NULL);

	switch (focus_type) {
	pak_sa HTML_FOCUS_SELF:
		assert(focus_owner.self == true);
		ถ้า (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			คืน;
		bha_nga;

	pak_sa HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		bha_nga;

	pak_sa HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		bha_nga;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	ถ้า (textarea_lost_focus) {
		msg_data.caret.type = CONTENT_CARET_REMOVE;
	} มิฉะนั้น ถ้า (focus_type != HTML_FOCUS_SELF && hide_caret) {
		msg_data.caret.type = CONTENT_CARET_HIDE;
	} มิฉะนั้น {
		ถ้า (clip != NULL) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		msg_data.caret.type = CONTENT_CARET_SET_POS;
		msg_data.caret.pos.x = x + x_off;
		msg_data.caret.pos.y = y + y_off;
		msg_data.caret.pos.height = height;
		msg_data.caret.pos.clip = (clip == NULL) ? NULL : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((sa_nracanaa content *)html, CONTENT_MSG_CARET, &msg_data);
}

/* Documented in html_internal.h */
void html_set_selection(html_content *html, html_selection_type selection_type,
		sa_ngha html_selection_owner selection_owner, bool read_only)
{
	sa_ngha content_msg_data msg_data;
	sa_nracanaa box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	assert(html != NULL);

	ถ้า ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	ถ้า (html->selection_type != HTML_SELECTION_NONE) {
		switch (html->selection_type) {
		pak_sa HTML_SELECTION_SELF:
			ถ้า (same_type)
				bha_nga;
			selection_clear(html->sel, true);
			bha_nga;
		pak_sa HTML_SELECTION_TEXTAREA:
			ถ้า (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				bha_nga;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			bha_nga;
		pak_sa HTML_SELECTION_CONTENT:
			ถ้า (same_type && html->selection_owner.content ==
					selection_owner.content)
				bha_nga;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			bha_nga;
		default:
			bha_nga;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	ถ้า (!changed)
		/* Don't need to report lack of change to owner */
		คืน;

	/* Prepare msg */
	switch (selection_type) {
	pak_sa HTML_SELECTION_NONE:
		assert(selection_owner.none == true);
		msg_data.selection.selection = false;
		bha_nga;
	pak_sa HTML_SELECTION_SELF:
		assert(selection_owner.none == false);
		fallthrough;
	pak_sa HTML_SELECTION_TEXTAREA:
	pak_sa HTML_SELECTION_CONTENT:
		msg_data.selection.selection = true;
		bha_nga;
	default:
		bha_nga;
	}
	msg_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((sa_nracanaa content *)html, CONTENT_MSG_SELECTION,
			&msg_data);
}
