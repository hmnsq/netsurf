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

	གལ་ཏེ (box->type == BOX_FLOAT_LEFT || box->type == BOX_FLOAT_RIGHT)
		style = box->children->style;
	དེ་མིན
		style = box->style;

	གལ་ཏེ (style == NULL)
		ཕྱིར BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	switch (cursor) {
	pak_sa CSS_CURSOR_AUTO:
		གལ་ཏེ (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} དེ་མིན གལ་ཏེ (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} དེ་མིན {
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

	ཕྱིར pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

static void html_box_drag_start(sa_nracanaa box *box, ཧྲིལ x, ཧྲིལ y)
{
	ཧྲིལ box_x, box_y;
	ཧྲིལ scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	གལ་ཏེ (box->scroll_x != NULL) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} དེ་མིན གལ་ཏེ (box->scroll_y != NULL) {
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
		browser_mouse_state mouse, ཧྲིལ x, ཧྲིལ y, ཧྲིལ dir)
{
	ཧྲིལ pixel_offset;
	sa_nracanaa box *box;
	ཧྲིལ dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	གལ་ཏེ (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	ཕྱིར idx;
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

	གལ་ཏེ (!dom_string_isequal(corestring_dom___ns_key_image_coords_node_data,
				key) || coords == NULL) {
		ཕྱིར;
	}

	switch (operation) {
	pak_sa DOM_NODE_CLONED:
		newcoords = calloc(1, parimaa_na(*newcoords));
		གལ་ཏེ (newcoords != NULL) {
			*newcoords = *coords;
			གལ་ཏེ (dom_node_set_user_data(dst,
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
			      ཧྲིལ x, ཧྲིལ y)
{
	ཧྲིལ scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	sa_nracanaa html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	sa_nracanaa box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	གལ་ཏེ (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} དེ་མིན {
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
			 ཧྲིལ x, ཧྲིལ y)
{
	sa_nracanaa box *box;
	ཧྲིལ box_x = 0;
	ཧྲིལ box_y = 0;
	const var_na *status;
	ཧྲིལ width, height;
	sa_nracanaa hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	assert(html->visible_select_menu != NULL);

	bw_drag_type = browser_window_get_drag_type(bw);
	གལ་ཏེ (bw_drag_type != DRAGGING_NONE && !mouse) {
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
	གལ་ཏེ (status != NULL) {
		/* set status if menu still open */
		sa_ngha content_msg_data msg_data;
		msg_data.explicit_status_text = status;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_STATUS,
				  &msg_data);
		ཕྱིར NSERROR_OK;
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
	ཕྱིར NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
static nserror
mouse_action_drag_selection(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    ཧྲིལ x, ཧྲིལ y)
{
	sa_nracanaa box *box;
	ཧྲིལ dir = -1;
	ཧྲིལ dx, dy;
	size_t idx;
	sa_ngha html_drag_owner drag_owner;
	ཧྲིལ pixel_offset;
	plot_font_style_t fstyle;

	གལ་ཏེ (!mouse) {
		/* End of selection drag */
		གལ་ཏེ (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		གལ་ཏེ (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, NULL);

		ཕྱིར NSERROR_OK;
	}

	གལ་ཏེ (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	གལ་ཏེ (box != NULL) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	ཕྱིར NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
static nserror
mouse_action_drag_scrollbar(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    ཧྲིལ x, ཧྲིལ y)
{
	sa_nracanaa scrollbar *scr;
	sa_nracanaa html_scrollbar_data *data;
	sa_nracanaa box *box;
	ཧྲིལ box_x = 0;
	ཧྲིལ box_y = 0;
	const var_na *status;
	ཧྲིལ scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	གལ་ཏེ (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	གལ་ཏེ (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} དེ་མིན {
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

	གལ་ཏེ (status != NULL) {
		sa_ngha content_msg_data msg_data;

		msg_data.explicit_status_text = status;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_STATUS,
				  &msg_data);
	}

	ཕྱིར NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
static nserror
mouse_action_drag_textarea(html_content *html,
			    sa_nracanaa browser_window *bw,
			    browser_mouse_state mouse,
			    ཧྲིལ x, ཧྲིལ y)
{
	sa_nracanaa box *box;
	ཧྲིལ box_x = 0;
	ཧྲིལ box_y = 0;

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
	ཕྱིར NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
static nserror
mouse_action_drag_content(html_content *html,
			  sa_nracanaa browser_window *bw,
			  browser_mouse_state mouse,
			  ཧྲིལ x, ཧྲིལ y)
{
	sa_nracanaa box *box;
	ཧྲིལ box_x = 0;
	ཧྲིལ box_y = 0;

	box = html->drag_owner.content;
	assert(box->object != NULL);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	ཕྱིར NSERROR_OK;
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
		ཧྲིལ pos_x;
		ཧྲིལ pos_y;
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
		ཧྲིལ box_x;
		ཧྲིལ box_y;
		const var_na *target;
	} gadget;

	/** title */
	const var_na *title;

	/** candidate box for drag operation */
	sa_nracanaa box *drag_candidate;

	/** scrollbar */
	sa_nracanaa {
		sa_nracanaa scrollbar *bar;
		ཧྲིལ mouse_x;
		ཧྲིལ mouse_y;
	} scroll;

	/** text in box */
	sa_nracanaa {
		sa_nracanaa box *box;
		ཧྲིལ box_x;
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
		      ཧྲིལ x, ཧྲིལ y,
		      sa_nracanaa mouse_action_state *man)
{
	sa_nracanaa box *box;
	ཧྲིལ box_x = 0;
	ཧྲིལ box_y = 0;

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
		གལ་ཏེ ((box->style != NULL) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			goto next_box;
		}

		གལ་ཏེ (box->node != NULL) {
			man->node = box->node;
		}

		གལ་ཏེ (box->object) {
			གལ་ཏེ (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} དེ་མིན {
				man->object = box->object;
			}
		}

		གལ་ཏེ (box->iframe) {
			man->iframe = box->iframe;
		}

		གལ་ཏེ (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		གལ་ཏེ (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		གལ་ཏེ (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			གལ་ཏེ (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		གལ་ཏེ (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		གལ་ཏེ ((box->scroll_x != NULL) ||
		    (box->scroll_y != NULL)) {
			ཧྲིལ padding_left;
			ཧྲིལ padding_right;
			ཧྲིལ padding_top;
			ཧྲིལ padding_bottom;

			གལ་ཏེ (man->drag_candidate == NULL) {
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

			གལ་ཏེ ((x > padding_left) &&
			    (x < padding_right) &&
			    (y > padding_top) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				གལ་ཏེ ((box->scroll_y != NULL) &&
				    (x > (padding_right - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_right - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_top;
					bha_nga;

				} དེ་མིན གལ་ཏེ ((box->scroll_x != NULL) &&
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

		གལ་ཏེ (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} སྐབས (box != NULL);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	assert(man->node != NULL);

	ཕྱིར NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
static nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    ཧྲིལ x, ཧྲིལ y,
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
		གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			གལ་ཏེ (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = NULL;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
			msg_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_MSG_SELECTMENU,
					  &msg_data);
		}
		bha_nga;

	pak_sa GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_input_element_set_checked(
				(dom_html_input_element *)(mas->gadget.control->node),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		bha_nga;

	pak_sa GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		bha_nga;

	pak_sa GADGET_IMAGE:
		/* This falls through to SUBMIT */
		གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
			sa_nracanaa image_input_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, parimaa_na(*coords));
			གལ་ཏེ (coords == NULL) {
				ཕྱིར NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			གལ་ཏེ (dom_node_set_user_data(
				mas->gadget.control->node,
				corestring_dom___ns_key_image_coords_node_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				ཕྱིར NSERROR_OK;
			}
			free(oldcoords);
		}
		fallthrough;

	pak_sa GADGET_SUBMIT:
		གལ་ཏེ (mas->gadget.control->form) {
			static var_na status_buffer[200];

			snprintf(status_buffer,
				 parimaa_na status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			གལ་ཏེ (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} དེ་མིན {
			mas->result.status = messages_get("FormBadSubmit");
		}
		bha_nga;

	pak_sa GADGET_TEXTBOX:
	pak_sa GADGET_PASSWORD:
	pak_sa GADGET_TEXTAREA:
		གལ་ཏེ (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} དེ་མིན {
			mas->result.status = messages_get("FormTextbox");
		}

		གལ་ཏེ (click &&
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

		གལ་ཏེ (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} དེ་མིན {
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
		གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
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

	ཕྱིར NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
static nserror
iframe_mouse_action(sa_nracanaa browser_window *bw,
		    browser_mouse_state mouse,
		    ཧྲིལ x, ཧྲིལ y,
		    sa_nracanaa mouse_action_state *mas)
{
	ཧྲིལ pos_x, pos_y;
	float scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} དེ་མིན {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	ཕྱིར NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
static nserror
html_object_mouse_action(html_content *html,
			 sa_nracanaa browser_window *bw,
			 browser_mouse_state mouse,
			 ཧྲིལ x, ཧྲིལ y,
			 sa_nracanaa mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	གལ་ཏེ (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		sa_ngha html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} དེ་མིན {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	ཕྱིར NSERROR_OK;
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
	གལ་ཏེ (scheme != NULL) {
		གལ་ཏེ (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	ཕྱིར is_js;
}


/**
 * process mouse activity on a link
 */
static nserror
link_mouse_action(html_content *html,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  ཧྲིལ x, ཧྲིལ y,
		  sa_nracanaa mouse_action_state *mas)
{
	nserror res;
	var_na *url_s = NULL;
	size_t url_l = 0;
	static var_na status_buffer[200];
	sa_ngha content_msg_data msg_data;

	གལ་ཏེ (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		གལ་ཏེ (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = NULL;
		}
	}

	གལ་ཏེ (mas->title) {
		snprintf(status_buffer,
			 parimaa_na status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} དེ་མིན {
		snprintf(status_buffer,
			 parimaa_na status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	གལ་ཏེ (url_s != NULL) {
		free(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((sa_nracanaa content *)html),
					BW_NAVIGATE_DOWNLOAD,
					NULL,
					NULL,
					NULL);

	} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		msg_data.savelink.url = mas->link.url;
		msg_data.savelink.title = mas->title;
		content_broadcast((sa_nracanaa content *)html,
				  CONTENT_MSG_SAVELINK,
				  &msg_data);

	} དེ་མིན གལ་ཏེ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		གལ་ཏེ (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} དེ་མིན {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	ཕྱིར NSERROR_OK;
}


static nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	གལ་ཏེ (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		sa_ngha html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, NULL);
	}

	ཕྱིར NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
static nserror
default_mouse_action(html_content *html,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  ཧྲིལ x, ཧྲིལ y,
		  sa_nracanaa mouse_action_state *mas)
{
	sa_nracanaa content *c = (sa_nracanaa content *)html;

	/* frame resizing */
	གལ་ཏེ (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		གལ་ཏེ (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		ཕྱིར default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	sa_ngha html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	གལ་ཏེ (click && html->focus_type != HTML_FOCUS_SELF) {
		sa_ngha html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, NULL);
	}
	གལ་ཏེ (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	གལ་ཏེ (mas->text.box) {
		ཧྲིལ pixel_offset;
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

		གལ་ཏེ (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			sa_ngha html_drag_owner drag_owner;

			གལ་ཏེ (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   NULL);
				mas->result.status = messages_get("Selecting");
			}

			གལ་ཏེ (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} དེ་མིན གལ་ཏེ (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			ཕྱིར default_mouse_action_focus(html, mouse);
		}

	} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	གལ་ཏེ (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} དེ་མིན གལ་ཏེ (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	གལ་ཏེ (mas->title) {
		mas->result.status = mas->title;
	}

	གལ་ཏེ (mouse & BROWSER_MOUSE_DRAG_1) {
		གལ་ཏེ (mouse & BROWSER_MOUSE_MOD_2) {
			sa_ngha content_msg_data msg_data;
			msg_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			msg_data.dragsave.content = NULL;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		} དེ་མིན {
			གལ་ཏེ (mas->drag_candidate == NULL) {
				browser_window_page_drag_start(bw, x, y);
			} དེ་མིན {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_DRAG_2) {
		གལ་ཏེ (mouse & BROWSER_MOUSE_MOD_2) {
			sa_ngha content_msg_data msg_data;
			msg_data.dragsave.type = CONTENT_SAVE_SOURCE;
			msg_data.dragsave.content = NULL;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		} དེ་མིན {
			གལ་ཏེ (mas->drag_candidate == NULL) {
				browser_window_page_drag_start(bw, x, y);
			} དེ་མིན {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	ཕྱིར default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
static nserror
mouse_action_drag_none(html_content *html,
		       sa_nracanaa browser_window *bw,
		       browser_mouse_state mouse,
		       ཧྲིལ x, ཧྲིལ y)
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
	གལ་ཏེ (res != NSERROR_OK) {
		ཕྱིར res;
	}

	གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} དེ་མིན གལ་ཏེ (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} དེ་མིན གལ་ཏེ (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} དེ་མིན གལ་ཏེ ((mas.object != NULL) && (mouse & BROWSER_MOUSE_MOD_2)) {

		གལ་ཏེ (mouse & BROWSER_MOUSE_DRAG_2) {
			msg_data.dragsave.type = CONTENT_SAVE_NATIVE;
			msg_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);

		} དེ་མིན གལ་ཏེ (mouse & BROWSER_MOUSE_DRAG_1) {
			msg_data.dragsave.type = CONTENT_SAVE_ORIG;
			msg_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, &msg_data);
		}

		/* \todo should have a drag-saving object msg */

	} དེ་མིན གལ་ཏེ (mas.iframe != NULL) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} དེ་མིན གལ་ཏེ (mas.html_object.box != NULL) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} དེ་མིན གལ་ཏེ (mas.link.url != NULL) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} དེ་མིན {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	གལ་ཏེ (res != NSERROR_OK) {
		ཕྱིར res;
	}

	/* send status and pointer message */
	གལ་ཏེ (mas.result.action != ACTION_NOSEND) {
		msg_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

		msg_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);
	}

	/* fire dom click event */
	གལ་ཏེ (mouse & BROWSER_MOUSE_CLICK_1) {
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
		གལ་ཏེ (path != NULL) {
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

	ཕྱིར res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(sa_nracanaa content *c,
			 sa_nracanaa browser_window *bw,
			 browser_mouse_state mouse,
			 ཧྲིལ x, ཧྲིལ y)
{
	ཕྱིར html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(sa_nracanaa content *c,
		  sa_nracanaa browser_window *bw,
		  browser_mouse_state mouse,
		  ཧྲིལ x, ཧྲིལ y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	གལ་ཏེ (html->visible_select_menu != NULL) {
		ཕྱིར mouse_action_select_menu(html, bw, mouse, x, y);
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

	གལ་ཏེ (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	ཕྱིར res;
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
	གལ་ཏེ (html->layout != NULL && html->layout->node != NULL) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->node, true, true, key);
	}

	switch (html->focus_type) {
	pak_sa HTML_FOCUS_CONTENT:
		ཕྱིར content_keypress(html->focus_owner.content->object, key);

	pak_sa HTML_FOCUS_TEXTAREA:
		གལ་ཏེ (box_textarea_keypress(html, html->focus_owner.textarea, key) == NSERROR_OK) {
			ཕྱིར true;
		} དེ་མིན {
			ཕྱིར false;
		}

	default:
		/* Deal with it below */
		bha_nga;
	}

	switch (key) {
	pak_sa NS_KEY_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		ཕྱིར true;

	pak_sa NS_KEY_CLEAR_SELECTION:
		selection_clear(sel, true);
		ཕྱིར true;

	pak_sa NS_KEY_SELECT_ALL:
		selection_select_all(sel);
		ཕྱིར true;

	pak_sa NS_KEY_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		ཕྱིར selection_clear(sel, true);
	}

	ཕྱིར false;
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

		གལ་ཏེ (html->reflowing == true) {
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
		ཧྲིལ x, ཧྲིལ y, ཧྲིལ height, const sa_nracanaa rect *clip)
{
	sa_ngha content_msg_data msg_data;
	ཧྲིལ x_off = 0;
	ཧྲིལ y_off = 0;
	sa_nracanaa rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	assert(html != NULL);

	switch (focus_type) {
	pak_sa HTML_FOCUS_SELF:
		assert(focus_owner.self == true);
		གལ་ཏེ (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			ཕྱིར;
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

	གལ་ཏེ (textarea_lost_focus) {
		msg_data.caret.type = CONTENT_CARET_REMOVE;
	} དེ་མིན གལ་ཏེ (focus_type != HTML_FOCUS_SELF && hide_caret) {
		msg_data.caret.type = CONTENT_CARET_HIDE;
	} དེ་མིན {
		གལ་ཏེ (clip != NULL) {
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

	གལ་ཏེ ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	གལ་ཏེ (html->selection_type != HTML_SELECTION_NONE) {
		switch (html->selection_type) {
		pak_sa HTML_SELECTION_SELF:
			གལ་ཏེ (same_type)
				bha_nga;
			selection_clear(html->sel, true);
			bha_nga;
		pak_sa HTML_SELECTION_TEXTAREA:
			གལ་ཏེ (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				bha_nga;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			bha_nga;
		pak_sa HTML_SELECTION_CONTENT:
			གལ་ཏེ (same_type && html->selection_owner.content ==
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

	གལ་ཏེ (!changed)
		/* Don't need to report lack of change to owner */
		ཕྱིར;

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
