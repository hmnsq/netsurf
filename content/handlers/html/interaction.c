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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>

#شامل <dom/dom.h>

#شامل "utils/corestrings.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/mouse.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/layout.h"
#شامل "netsurf/keypress.h"
#شامل "content/hlcache.h"
#شامل "content/textsearch.h"
#شامل "desktop/browser_history.h"
#شامل "desktop/frames.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/selection.h"
#شامل "desktop/textarea.h"
#شامل "javascript/js.h"
#شامل "desktop/gui_internal.h"

#شامل "html/box.h"
#شامل "html/box_textarea.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"
#شامل "html/private.h"
#شامل "html/imagemap.h"
#شامل "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

ایستا browser_pointer_shape get_pointer_shape(ساختار box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	شمارش css_cursor_e cursor;
	lwc_string **cursor_uris;

	اگر (box->type == BOX_FLOAT_چپ || box->type == BOX_FLOAT_راست)
		style = box->children->style;
	وگرنه
		style = box->style;

	اگر (style == پوچ)
		بازگشت BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	انتخاب (cursor) {
	حالت CSS_CURSOR_AUTO:
		اگر (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} وگرنه اگر (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} وگرنه {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		بشکن;
	حالت CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		بشکن;
	حالت CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		بشکن;
	حالت CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		بشکن;
	حالت CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_راست;
		بشکن;
	حالت CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_چپ;
		بشکن;
	حالت CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		بشکن;
	حالت CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		بشکن;
	حالت CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		بشکن;
	حالت CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		بشکن;
	حالت CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		بشکن;
	حالت CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		بشکن;
	حالت CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		بشکن;
	حالت CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		بشکن;
	حالت CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		بشکن;
	حالت CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		بشکن;
	پیشفرض:
		pointer = BROWSER_POINTER_DEFAULT;
		بشکن;
	}

	بازگشت pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

ایستا تهی html_box_drag_start(ساختار box *box, صحیح x, صحیح y)
{
	صحیح box_x, box_y;
	صحیح scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	اگر (box->scroll_x != پوچ) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[بالاترین_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} وگرنه اگر (box->scroll_y != پوچ) {
		scroll_mouse_x = x - (box_x + box->padding[چپ_2] +
				box->width + box->padding[راست_2] -
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
ایستا size_t html_selection_drag_end(ساختار html_content *html,
		browser_mouse_state mouse, صحیح x, صحیح y, صحیح dir)
{
	صحیح pixel_offset;
	ساختار box *box;
	صحیح dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	اگر (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	بازگشت idx;
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
ایستا تهی
html__image_coords_dom_user_data_handler(dom_گره_operation operation,
					 dom_string *کلید,
					 تهی *_data,
					 ساختار dom_گره *src,
					 ساختار dom_گره *dst)
{
	ساختار image_درون_داد_coords *oldcoords, *coords = _data, *newcoords;

	اگر (!dom_string_isequal(corestring_dom_ns_کلید_image_coords_گره_data,
				کلید) || coords == پوچ) {
		بازگشت;
	}

	انتخاب (operation) {
	حالت DOM_گره_CLONED:
		newcoords = calloc(1, اندازه(*newcoords));
		اگر (newcoords != پوچ) {
			*newcoords = *coords;
			اگر (dom_گره_set_user_data(dst,
				 corestring_dom_ns_کلید_image_coords_گره_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				آزاد(oldcoords);
			}
		}
		بشکن;

	حالت DOM_گره_DELETED:
		آزاد(coords);
		بشکن;

	حالت DOM_گره_RENAMED:
	حالت DOM_گره_IMPORTED:
	حالت DOM_گره_ADOPTED:
		بشکن;

	پیشفرض:
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
ایستا تهی
html_overflow_scroll_drag_end(ساختار scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      صحیح x, صحیح y)
{
	صحیح scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	ساختار html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	ساختار box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	اگر (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[بالاترین_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} وگرنه {
		scroll_mouse_x = x - (box_x + box->padding[چپ_2] +
				box->width + box->padding[راست_2] -
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
ایستا nserror
mouse_action_select_menu(html_content *html,
			 ساختار browser_window *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	ساختار box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;
	ثابت نویسه *status;
	صحیح width, height;
	ساختار hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	أكد(html->visible_select_menu != پوچ);

	bw_drag_type = browser_window_get_drag_type(bw);
	اگر (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[چپ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[بالاترین_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	اگر (status != پوچ) {
		/* set status if menu still open */
		اجتماع content_پیام_data پیام_data;
		پیام_data.explicit_status_text = status;
		content_broadcast((ساختار content *)html,
				  CONTENT_پیام_STATUS,
				  &پیام_data);
		بازگشت NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = پوچ;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	بازگشت NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
ایستا nserror
mouse_action_drag_selection(html_content *html,
			    ساختار browser_window *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساختار box *box;
	صحیح dir = -1;
	صحیح dx, dy;
	size_t idx;
	اجتماع html_drag_owner drag_owner;
	صحیح pixel_offset;
	plot_font_style_t fstyle;

	اگر (!mouse) {
		/* End of selection drag */
		اگر (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		اگر (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, پوچ);

		بازگشت NSERROR_OK;
	}

	اگر (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	اگر (box != پوچ) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	بازگشت NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
ایستا nserror
mouse_action_drag_scrollbar(html_content *html,
			    ساختار browser_window *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساختار scrollbar *scr;
	ساختار html_scrollbar_data *data;
	ساختار box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;
	ثابت نویسه *status;
	صحیح scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	اگر (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	اگر (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[بالاترین_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} وگرنه {
		scroll_mouse_x = x - (box_x + box->padding[چپ_2] +
				      box->width + box->padding[راست_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	اگر (status != پوچ) {
		اجتماع content_پیام_data پیام_data;

		پیام_data.explicit_status_text = status;
		content_broadcast((ساختار content *)html,
				  CONTENT_پیام_STATUS,
				  &پیام_data);
	}

	بازگشت NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
ایستا nserror
mouse_action_drag_textarea(html_content *html,
			    ساختار browser_window *bw,
			    browser_mouse_state mouse,
			    صحیح x, صحیح y)
{
	ساختار box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	box = html->drag_owner.textarea;

	أكد(box->gadget != پوچ);
	أكد(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	بازگشت NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
ایستا nserror
mouse_action_drag_content(html_content *html,
			  ساختار browser_window *bw,
			  browser_mouse_state mouse,
			  صحیح x, صحیح y)
{
	ساختار box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	box = html->drag_owner.content;
	أكد(box->object != پوچ);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	بازگشت NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
ساختار mouse_action_state {
	ساختار {
		ثابت نویسه *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		شمارش {
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
	ساختار dom_گره *گره;

	/** html object */
	ساختار {
		ساختار box *box;
		صحیح pos_x;
		صحیح pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	ساختار browser_window *iframe;

	/** link either from href or imagemap */
	ساختار {
		ساختار box *box;
		nsurl *url;
		ثابت نویسه *target;
		bool is_imagemap;
	} link;

	/** gadget */
	ساختار {
		ساختار form_control *control;
		ساختار box *box;
		صحیح box_x;
		صحیح box_y;
		ثابت نویسه *target;
	} gadget;

	/** title */
	ثابت نویسه *title;

	/** candidate box for drag operation */
	ساختار box *drag_candidate;

	/** scrollbar */
	ساختار {
		ساختار scrollbar *bar;
		صحیح mouse_x;
		صحیح mouse_y;
	} scroll;

	/** text in box */
	ساختار {
		ساختار box *box;
		صحیح box_x;
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
ایستا nserror
get_mouse_action_گره(html_content *html,
		      صحیح x, صحیح y,
		      ساختار mouse_action_state *man)
{
	ساختار box *box;
	صحیح box_x = 0;
	صحیح box_y = 0;

	/* initialise the mouse action state data */
	memset(man, 0, اندازه(ساختار mouse_action_state));
	man->گره = html->layout->گره; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[چپ_2];
	box_y = box->margin[بالاترین_2];

	انجامبده {
		/* skip hidden boxes */
		اگر ((box->style != پوچ) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			برو next_box;
		}

		اگر (box->گره != پوچ) {
			man->گره = box->گره;
		}

		اگر (box->object) {
			اگر (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} وگرنه {
				man->object = box->object;
			}
		}

		اگر (box->iframe) {
			man->iframe = box->iframe;
		}

		اگر (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		اگر (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		اگر (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			اگر (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		اگر (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		اگر ((box->scroll_x != پوچ) ||
		    (box->scroll_y != پوچ)) {
			صحیح padding_چپ;
			صحیح padding_راست;
			صحیح padding_بالاترین;
			صحیح padding_bottom;

			اگر (man->drag_candidate == پوچ) {
				man->drag_candidate = box;
			}

			padding_چپ = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_راست = padding_چپ + box->padding[چپ_2] +
					box->width + box->padding[راست_2];
			padding_بالاترین = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_بالاترین + box->padding[بالاترین_2] +
					box->height + box->padding[BOTTOM];

			اگر ((x > padding_چپ) &&
			    (x < padding_راست) &&
			    (y > padding_بالاترین) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				اگر ((box->scroll_y != پوچ) &&
				    (x > (padding_راست - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_راست - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_بالاترین;
					بشکن;

				} وگرنه اگر ((box->scroll_x != پوچ) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_چپ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					بشکن;
				}
			}
		}

		اگر (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} وقتی (box != پوچ);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	أكد(man->گره != پوچ);

	بازگشت NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
ایستا nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y,
		    ساختار mouse_action_state *mas)
{
	ساختار content *c = (ساختار content *)html;
	textarea_mouse_status ta_status;
	اجتماع content_پیام_data پیام_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	انتخاب (mas->gadget.control->type) {
	حالت GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		اگر (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			اگر (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = پوچ;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} وگرنه اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			پیام_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_پیام_SELECTMENU,
					  &پیام_data);
		}
		بشکن;

	حالت GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_درون_داد_element_set_checked(
				(dom_html_درون_داد_element *)(mas->gadget.control->گره),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		بشکن;

	حالت GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		بشکن;

	حالت GADGET_IMAGE:
		/* This falls through to SUBMIT */
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			ساختار image_درون_داد_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = calloc(1, اندازه(*coords));
			اگر (coords == پوچ) {
				بازگشت NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			اگر (dom_گره_set_user_data(
				mas->gadget.control->گره,
				corestring_dom_ns_کلید_image_coords_گره_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				بازگشت NSERROR_OK;
			}
			آزاد(oldcoords);
		}
		fallthrough;

	حالت GADGET_SUBMIT:
		اگر (mas->gadget.control->form) {
			ایستا نویسه status_buffer[200];

			snprintf(status_buffer,
				 اندازه status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			اگر (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} وگرنه {
			mas->result.status = messages_get("FormBadSubmit");
		}
		بشکن;

	حالت GADGET_TEXTBOX:
	حالت GADGET_PASSWORD:
	حالت GADGET_TEXTAREA:
		اگر (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} وگرنه {
			mas->result.status = messages_get("FormTextbox");
		}

		اگر (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			اجتماع html_selection_owner sel_owner;
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

		اگر (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} وگرنه {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		بشکن;

	حالت GADGET_HIDDEN:
		/* not possible: no box generated */
		بشکن;

	حالت GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		بشکن;

	حالت GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		اگر (mouse & BROWSER_MOUSE_CLICK_1) {
			پیام_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_پیام_GADGETCLICK,
					  &پیام_data);
		}
		بشکن;

	حالت GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		بشکن;
	}

	بازگشت NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
ایستا nserror
iframe_mouse_action(ساختار browser_window *bw,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y,
		    ساختار mouse_action_state *mas)
{
	صحیح pos_x, pos_y;
	اعشاری scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	اگر (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} وگرنه {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	بازگشت NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
ایستا nserror
html_object_mouse_action(html_content *html,
			 ساختار browser_window *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y,
			 ساختار mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	اگر (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		اجتماع html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} وگرنه {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	بازگشت NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
ایستا bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme != پوچ) {
		اگر (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	بازگشت is_js;
}


/**
 * process mouse activity on a link
 */
ایستا nserror
link_mouse_action(html_content *html,
		  ساختار browser_window *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y,
		  ساختار mouse_action_state *mas)
{
	nserror res;
	نویسه *url_s = پوچ;
	size_t url_l = 0;
	ایستا نویسه status_buffer[200];
	اجتماع content_پیام_data پیام_data;

	اگر (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		اگر (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = پوچ;
		}
	}

	اگر (mas->title) {
		snprintf(status_buffer,
			 اندازه status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} وگرنه {
		snprintf(status_buffer,
			 اندازه status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	اگر (url_s != پوچ) {
		آزاد(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	اگر (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((ساختار content *)html),
					BW_NAVIGATE_DOWNLOAD,
					پوچ,
					پوچ,
					پوچ);

	} وگرنه اگر (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		پیام_data.savelink.url = mas->link.url;
		پیام_data.savelink.title = mas->title;
		content_broadcast((ساختار content *)html,
				  CONTENT_پیام_SAVELINK,
				  &پیام_data);

	} وگرنه اگر (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		اگر (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} وگرنه {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	بازگشت NSERROR_OK;
}


ایستا nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	اگر (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		اجتماع html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, پوچ);
	}

	بازگشت NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
ایستا nserror
default_mouse_action(html_content *html,
		  ساختار browser_window *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y,
		  ساختار mouse_action_state *mas)
{
	ساختار content *c = (ساختار content *)html;

	/* frame resizing */
	اگر (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		اگر (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		بازگشت default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	اجتماع html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	اگر (click && html->focus_type != HTML_FOCUS_SELF) {
		اجتماع html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, پوچ);
	}
	اگر (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mas->text.box) {
		صحیح pixel_offset;
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

		اگر (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			اجتماع html_drag_owner drag_owner;

			اگر (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   پوچ);
				mas->result.status = messages_get("Selecting");
			}

			اگر (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} وگرنه اگر (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			بازگشت default_mouse_action_focus(html, mouse);
		}

	} وگرنه اگر (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	اگر (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} وگرنه اگر (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	اگر (mas->title) {
		mas->result.status = mas->title;
	}

	اگر (mouse & BROWSER_MOUSE_DRAG_1) {
		اگر (mouse & BROWSER_MOUSE_MOD_2) {
			اجتماع content_پیام_data پیام_data;
			پیام_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			پیام_data.dragsave.content = پوچ;
			content_broadcast(c, CONTENT_پیام_DRAGSAVE, &پیام_data);
		} وگرنه {
			اگر (mas->drag_candidate == پوچ) {
				browser_window_page_drag_start(bw, x, y);
			} وگرنه {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} وگرنه اگر (mouse & BROWSER_MOUSE_DRAG_2) {
		اگر (mouse & BROWSER_MOUSE_MOD_2) {
			اجتماع content_پیام_data پیام_data;
			پیام_data.dragsave.type = CONTENT_SAVE_SOURCE;
			پیام_data.dragsave.content = پوچ;
			content_broadcast(c, CONTENT_پیام_DRAGSAVE, &پیام_data);
		} وگرنه {
			اگر (mas->drag_candidate == پوچ) {
				browser_window_page_drag_start(bw, x, y);
			} وگرنه {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	بازگشت default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
ایستا nserror
mouse_action_drag_none(html_content *html,
		       ساختار browser_window *bw,
		       browser_mouse_state mouse,
		       صحیح x, صحیح y)
{
	nserror res;
	ساختار content *c = (ساختار content *)html;
	اجتماع content_پیام_data پیام_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	ایستا ساختار mouse_action_state mas;

	res = get_mouse_action_گره(html, x, y, &mas);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	اگر (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} وگرنه اگر (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} وگرنه اگر (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} وگرنه اگر (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} وگرنه اگر ((mas.object != پوچ) && (mouse & BROWSER_MOUSE_MOD_2)) {

		اگر (mouse & BROWSER_MOUSE_DRAG_2) {
			پیام_data.dragsave.type = CONTENT_SAVE_NATIVE;
			پیام_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_پیام_DRAGSAVE, &پیام_data);

		} وگرنه اگر (mouse & BROWSER_MOUSE_DRAG_1) {
			پیام_data.dragsave.type = CONTENT_SAVE_ORIG;
			پیام_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_پیام_DRAGSAVE, &پیام_data);
		}

		/* \todo should have a drag-saving object msg */

	} وگرنه اگر (mas.iframe != پوچ) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} وگرنه اگر (mas.html_object.box != پوچ) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} وگرنه اگر (mas.link.url != پوچ) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} وگرنه {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	/* send status and pointer message */
	اگر (mas.result.action != ACTION_NOSEND) {
		پیام_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_پیام_STATUS, &پیام_data);

		پیام_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_پیام_POINTER, &پیام_data);
	}

	/* fire dom click event */
	اگر (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.گره, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	انتخاب (mas.result.action) {
	حالت ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		بشکن;

	حالت ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				پوچ,
				پوچ,
				پوچ);
		بشکن;

	حالت ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		اگر (path != پوچ) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		بشکن;

	حالت ACTION_BACK:
		res = browser_window_history_back(bw, false);
		بشکن;

	حالت ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		بشکن;

	حالت ACTION_NOSEND:
	حالت ACTION_NONE:
		res = NSERROR_OK;
		بشکن;
	}

	بازگشت res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(ساختار content *c,
			 ساختار browser_window *bw,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	بازگشت html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(ساختار content *c,
		  ساختار browser_window *bw,
		  browser_mouse_state mouse,
		  صحیح x, صحیح y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	اگر (html->visible_select_menu != پوچ) {
		بازگشت mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	انتخاب (html->drag_type) {
	حالت HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		بشکن;

	حالت HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		بشکن;

	حالت HTML_DRAG_TEXTAREA_SELECTION:
	حالت HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		بشکن;

	حالت HTML_DRAG_CONTENT_SELECTION:
	حالت HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		بشکن;

	حالت HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		بشکن;

	پیشفرض:
		/* Unknown content related drag type */
		أكد(0 && "Unknown content related drag type");
	}

	اگر (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	بازگشت res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(ساختار content *c, uint32_t کلید)
{
	html_content *html = (html_content *) c;
	ساختار selection *sel = html->sel;

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
	اگر (html->layout != پوچ && html->layout->گره != پوچ) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->گره, true, true, کلید);
	}

	انتخاب (html->focus_type) {
	حالت HTML_FOCUS_CONTENT:
		بازگشت content_keypress(html->focus_owner.content->object, کلید);

	حالت HTML_FOCUS_TEXTAREA:
		اگر (box_textarea_keypress(html, html->focus_owner.textarea, کلید) == NSERROR_OK) {
			بازگشت true;
		} وگرنه {
			بازگشت false;
		}

	پیشفرض:
		/* Deal with it below */
		بشکن;
	}

	انتخاب (کلید) {
	حالت NS_کلید_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		بازگشت true;

	حالت NS_کلید_CLEAR_SELECTION:
		selection_clear(sel, true);
		بازگشت true;

	حالت NS_کلید_SELECT_ALL:
		selection_select_all(sel);
		بازگشت true;

	حالت NS_کلید_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		بازگشت selection_clear(sel, true);
	}

	بازگشت false;
}


/**
 * Callback for in-page scrollbars.
 */
تهی html_overflow_scroll_callback(تهی *client_data,
		ساختار scrollbar_پیام_data *scrollbar_data)
{
	ساختار html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	ساختار box *box = data->box;
	اجتماع content_پیام_data پیام_data;
	html_drag_type drag_type;
	اجتماع html_drag_owner drag_owner;

	انتخاب(scrollbar_data->پیام) {
	حالت SCROLLBAR_پیام_MOVED:

		اگر (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			بشکن;
		}

		html__redraw_a_box(html, box);
		بشکن;
	حالت SCROLLBAR_پیام_SCROLL_START:
	{
		ساختار rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		بشکن;
	حالت SCROLLBAR_پیام_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, پوچ);

		پیام_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_پیام_POINTER, &پیام_data);
		بشکن;
	}
}


/* Documented in html_internal.h */
تهی html_set_drag_type(html_content *html, html_drag_type drag_type,
		اجتماع html_drag_owner drag_owner, ثابت ساختار rect *rect)
{
	اجتماع content_پیام_data پیام_data;

	أكد(html != پوچ);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	انتخاب (drag_type) {
	حالت HTML_DRAG_NONE:
		أكد(drag_owner.no_owner == true);
		پیام_data.drag.type = CONTENT_DRAG_NONE;
		بشکن;

	حالت HTML_DRAG_SCROLLBAR:
	حالت HTML_DRAG_TEXTAREA_SCROLLBAR:
	حالت HTML_DRAG_CONTENT_SCROLL:
		پیام_data.drag.type = CONTENT_DRAG_SCROLL;
		بشکن;

	حالت HTML_DRAG_SELECTION:
		أكد(drag_owner.no_owner == true);
		fallthrough;
	حالت HTML_DRAG_TEXTAREA_SELECTION:
	حالت HTML_DRAG_CONTENT_SELECTION:
		پیام_data.drag.type = CONTENT_DRAG_SELECTION;
		بشکن;
	}
	پیام_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((ساختار content *)html, CONTENT_پیام_DRAG, &پیام_data);
}

/* Documented in html_internal.h */
تهی html_set_focus(html_content *html, html_focus_type focus_type,
		اجتماع html_focus_owner focus_owner, bool hide_caret,
		صحیح x, صحیح y, صحیح height, ثابت ساختار rect *clip)
{
	اجتماع content_پیام_data پیام_data;
	صحیح x_off = 0;
	صحیح y_off = 0;
	ساختار rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	أكد(html != پوچ);

	انتخاب (focus_type) {
	حالت HTML_FOCUS_SELF:
		أكد(focus_owner.self == true);
		اگر (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			بازگشت;
		بشکن;

	حالت HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		بشکن;

	حالت HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		بشکن;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	اگر (textarea_lost_focus) {
		پیام_data.caret.type = CONTENT_CARET_REMOVE;
	} وگرنه اگر (focus_type != HTML_FOCUS_SELF && hide_caret) {
		پیام_data.caret.type = CONTENT_CARET_HIDE;
	} وگرنه {
		اگر (clip != پوچ) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		پیام_data.caret.type = CONTENT_CARET_SET_POS;
		پیام_data.caret.pos.x = x + x_off;
		پیام_data.caret.pos.y = y + y_off;
		پیام_data.caret.pos.height = height;
		پیام_data.caret.pos.clip = (clip == پوچ) ? پوچ : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((ساختار content *)html, CONTENT_پیام_CARET, &پیام_data);
}

/* Documented in html_internal.h */
تهی html_set_selection(html_content *html, html_selection_type selection_type,
		اجتماع html_selection_owner selection_owner, bool read_only)
{
	اجتماع content_پیام_data پیام_data;
	ساختار box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	أكد(html != پوچ);

	اگر ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	اگر (html->selection_type != HTML_SELECTION_NONE) {
		انتخاب (html->selection_type) {
		حالت HTML_SELECTION_SELF:
			اگر (same_type)
				بشکن;
			selection_clear(html->sel, true);
			بشکن;
		حالت HTML_SELECTION_TEXTAREA:
			اگر (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				بشکن;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			بشکن;
		حالت HTML_SELECTION_CONTENT:
			اگر (same_type && html->selection_owner.content ==
					selection_owner.content)
				بشکن;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			بشکن;
		پیشفرض:
			بشکن;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	اگر (!changed)
		/* Don't need to report lack of change to owner */
		بازگشت;

	/* Prepare msg */
	انتخاب (selection_type) {
	حالت HTML_SELECTION_NONE:
		أكد(selection_owner.none == true);
		پیام_data.selection.selection = false;
		بشکن;
	حالت HTML_SELECTION_SELF:
		أكد(selection_owner.none == false);
		fallthrough;
	حالت HTML_SELECTION_TEXTAREA:
	حالت HTML_SELECTION_CONTENT:
		پیام_data.selection.selection = true;
		بشکن;
	پیشفرض:
		بشکن;
	}
	پیام_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((ساختار content *)html, CONTENT_پیام_SELECTION,
			&پیام_data);
}
