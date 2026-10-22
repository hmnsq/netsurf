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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/mouse.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/layout.h"
#अन्तर्भाव "netsurf/keypress.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "content/textsearch.h"
#अन्तर्भाव "desktop/browser_history.h"
#अन्तर्भाव "desktop/frames.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/selection.h"
#अन्तर्भाव "desktop/textarea.h"
#अन्तर्भाव "javascript/js.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_textarea.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/imagemap.h"
#अन्तर्भाव "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

स्थिर browser_pointer_shape get_pointer_shape(संरचना box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	गणना css_cursor_e cursor;
	lwc_string **cursor_uris;

	जर (box->type == BOX_FLOAT_डावा || box->type == BOX_FLOAT_उजवा)
		style = box->children->style;
	अन्यथा
		style = box->style;

	जर (style == रिक्त)
		परत BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	निवड (cursor) {
	स्थिती CSS_CURSOR_AUTO:
		जर (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} अन्यथा जर (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} अन्यथा {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		थांबा;
	स्थिती CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		थांबा;
	स्थिती CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		थांबा;
	स्थिती CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		थांबा;
	स्थिती CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_उजवा;
		थांबा;
	स्थिती CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_डावा;
		थांबा;
	स्थिती CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		थांबा;
	स्थिती CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		थांबा;
	स्थिती CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		थांबा;
	स्थिती CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		थांबा;
	स्थिती CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		थांबा;
	स्थिती CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		थांबा;
	स्थिती CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		थांबा;
	स्थिती CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		थांबा;
	स्थिती CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		थांबा;
	स्थिती CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		थांबा;
	पूर्वनिर्धारित:
		pointer = BROWSER_POINTER_DEFAULT;
		थांबा;
	}

	परत pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

स्थिर शून्य html_box_drag_start(संरचना box *box, पूर्णांक x, पूर्णांक y)
{
	पूर्णांक box_x, box_y;
	पूर्णांक scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	जर (box->scroll_x != रिक्त) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा जर (box->scroll_y != रिक्त) {
		scroll_mouse_x = x - (box_x + box->padding[डावा_2] +
				box->width + box->padding[उजवा_2] -
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
स्थिर माप_प्रकार html_selection_drag_end(संरचना html_content *html,
		browser_mouse_state mouse, पूर्णांक x, पूर्णांक y, पूर्णांक dir)
{
	पूर्णांक pixel_offset;
	संरचना box *box;
	पूर्णांक dx, dy;
	माप_प्रकार idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	जर (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->length,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	परत idx;
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
स्थिर शून्य
html__image_coords_dom_user_data_handler(dom_गाठ_operation operation,
					 dom_string *किल्ली,
					 शून्य *_data,
					 संरचना dom_गाठ *src,
					 संरचना dom_गाठ *dst)
{
	संरचना image_आदान_coords *oldcoords, *coords = _data, *newcoords;

	जर (!dom_string_isequal(corestring_dom_ns_किल्ली_image_coords_गाठ_data,
				किल्ली) || coords == रिक्त) {
		परत;
	}

	निवड (operation) {
	स्थिती DOM_गाठ_CLONED:
		newcoords = सुस्मृति(1, आकार(*newcoords));
		जर (newcoords != रिक्त) {
			*newcoords = *coords;
			जर (dom_गाठ_set_user_data(dst,
				 corestring_dom_ns_किल्ली_image_coords_गाठ_data,
				 newcoords,
				 html__image_coords_dom_user_data_handler,
				 &oldcoords) == DOM_NO_ERR) {
				मुक्त(oldcoords);
			}
		}
		थांबा;

	स्थिती DOM_गाठ_DELETED:
		मुक्त(coords);
		थांबा;

	स्थिती DOM_गाठ_RENAMED:
	स्थिती DOM_गाठ_IMPORTED:
	स्थिती DOM_गाठ_ADOPTED:
		थांबा;

	पूर्वनिर्धारित:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		प्रतिज्ञा(0);
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
स्थिर शून्य
html_overflow_scroll_drag_end(संरचना scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      पूर्णांक x, पूर्णांक y)
{
	पूर्णांक scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	संरचना html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	संरचना box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	जर (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[डावा_2] +
				box->width + box->padding[उजवा_2] -
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
स्थिर nserror
mouse_action_select_menu(html_content *html,
			 संरचना browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	संरचना box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;
	स्थिरांक अक्षर *status;
	पूर्णांक width, height;
	संरचना hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	प्रतिज्ञा(html->visible_select_menu != रिक्त);

	bw_drag_type = browser_window_get_drag_type(bw);
	जर (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[डावा_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[शीर्ष_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	जर (status != रिक्त) {
		/* set status if menu still open */
		संघ content_संदेश_data संदेश_data;
		संदेश_data.explicit_status_text = status;
		content_broadcast((संरचना content *)html,
				  CONTENT_संदेश_STATUS,
				  &संदेश_data);
		परत NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = रिक्त;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	परत NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
स्थिर nserror
mouse_action_drag_selection(html_content *html,
			    संरचना browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	संरचना box *box;
	पूर्णांक dir = -1;
	पूर्णांक dx, dy;
	माप_प्रकार idx;
	संघ html_drag_owner drag_owner;
	पूर्णांक pixel_offset;
	plot_font_style_t fstyle;

	जर (!mouse) {
		/* End of selection drag */
		जर (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		जर (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, रिक्त);

		परत NSERROR_OK;
	}

	जर (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	जर (box != रिक्त) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->length,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	परत NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
स्थिर nserror
mouse_action_drag_scrollbar(html_content *html,
			    संरचना browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	संरचना scrollbar *scr;
	संरचना html_scrollbar_data *data;
	संरचना box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;
	स्थिरांक अक्षर *status;
	पूर्णांक scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	जर (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	data = scrollbar_get_data(scr);

	box = data->box;

	box_coords(box, &box_x, &box_y);

	जर (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[डावा_2] +
				      box->width + box->padding[उजवा_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	जर (status != रिक्त) {
		संघ content_संदेश_data संदेश_data;

		संदेश_data.explicit_status_text = status;
		content_broadcast((संरचना content *)html,
				  CONTENT_संदेश_STATUS,
				  &संदेश_data);
	}

	परत NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
स्थिर nserror
mouse_action_drag_textarea(html_content *html,
			    संरचना browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	संरचना box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	box = html->drag_owner.textarea;

	प्रतिज्ञा(box->gadget != रिक्त);
	प्रतिज्ञा(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->data.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	परत NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
स्थिर nserror
mouse_action_drag_content(html_content *html,
			  संरचना browser_window *bw,
			  browser_mouse_state mouse,
			  पूर्णांक x, पूर्णांक y)
{
	संरचना box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	box = html->drag_owner.content;
	प्रतिज्ञा(box->object != रिक्त);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	परत NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
संरचना mouse_action_state {
	संरचना {
		स्थिरांक अक्षर *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		गणना {
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
	संरचना dom_गाठ *गाठ;

	/** html object */
	संरचना {
		संरचना box *box;
		पूर्णांक pos_x;
		पूर्णांक pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	संरचना browser_window *iframe;

	/** link either from href or imagemap */
	संरचना {
		संरचना box *box;
		nsurl *url;
		स्थिरांक अक्षर *target;
		bool is_imagemap;
	} link;

	/** gadget */
	संरचना {
		संरचना form_control *control;
		संरचना box *box;
		पूर्णांक box_x;
		पूर्णांक box_y;
		स्थिरांक अक्षर *target;
	} gadget;

	/** title */
	स्थिरांक अक्षर *title;

	/** candidate box for drag operation */
	संरचना box *drag_candidate;

	/** scrollbar */
	संरचना {
		संरचना scrollbar *bar;
		पूर्णांक mouse_x;
		पूर्णांक mouse_y;
	} scroll;

	/** text in box */
	संरचना {
		संरचना box *box;
		पूर्णांक box_x;
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
स्थिर nserror
get_mouse_action_गाठ(html_content *html,
		      पूर्णांक x, पूर्णांक y,
		      संरचना mouse_action_state *man)
{
	संरचना box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	/* initialise the mouse action state data */
	स_रखो(man, 0, आकार(संरचना mouse_action_state));
	man->गाठ = html->layout->गाठ; /* Default dom node to the <HTML> */
	man->result.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[डावा_2];
	box_y = box->margin[शीर्ष_2];

	करा {
		/* skip hidden boxes */
		जर ((box->style != रिक्त) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			जा next_box;
		}

		जर (box->गाठ != रिक्त) {
			man->गाठ = box->गाठ;
		}

		जर (box->object) {
			जर (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} अन्यथा {
				man->object = box->object;
			}
		}

		जर (box->iframe) {
			man->iframe = box->iframe;
		}

		जर (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		जर (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		जर (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			जर (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		जर (box->title) {
			man->title = box->title;
		}

		man->result.pointer = get_pointer_shape(box, false);

		जर ((box->scroll_x != रिक्त) ||
		    (box->scroll_y != रिक्त)) {
			पूर्णांक padding_डावा;
			पूर्णांक padding_उजवा;
			पूर्णांक padding_शीर्ष;
			पूर्णांक padding_bottom;

			जर (man->drag_candidate == रिक्त) {
				man->drag_candidate = box;
			}

			padding_डावा = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_उजवा = padding_डावा + box->padding[डावा_2] +
					box->width + box->padding[उजवा_2];
			padding_शीर्ष = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_शीर्ष + box->padding[शीर्ष_2] +
					box->height + box->padding[BOTTOM];

			जर ((x > padding_डावा) &&
			    (x < padding_उजवा) &&
			    (y > padding_शीर्ष) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				जर ((box->scroll_y != रिक्त) &&
				    (x > (padding_उजवा - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_उजवा - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_शीर्ष;
					थांबा;

				} अन्यथा जर ((box->scroll_x != रिक्त) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_डावा;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					थांबा;
				}
			}
		}

		जर (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	next_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} पर्यंत (box != रिक्त);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	प्रतिज्ञा(man->गाठ != रिक्त);

	परत NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
स्थिर nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    पूर्णांक x, पूर्णांक y,
		    संरचना mouse_action_state *mas)
{
	संरचना content *c = (संरचना content *)html;
	textarea_mouse_status ta_status;
	संघ content_संदेश_data संदेश_data;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	निवड (mas->gadget.control->type) {
	स्थिती GADGET_SELECT:
		mas->result.status = messages_get("FormSelect");
		mas->result.pointer = BROWSER_POINTER_MENU;
		जर (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			जर (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = रिक्त;
			}
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
		} अन्यथा जर (mouse & BROWSER_MOUSE_CLICK_1) {
			संदेश_data.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_संदेश_SELECTMENU,
					  &संदेश_data);
		}
		थांबा;

	स्थिती GADGET_CHECKBOX:
		mas->result.status = messages_get("FormCheckbox");
		जर (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_आदान_element_set_checked(
				(dom_html_आदान_element *)(mas->gadget.control->गाठ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		थांबा;

	स्थिती GADGET_RADIO:
		mas->result.status = messages_get("FormRadio");
		जर (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		थांबा;

	स्थिती GADGET_IMAGE:
		/* This falls through to SUBMIT */
		जर (mouse & BROWSER_MOUSE_CLICK_1) {
			संरचना image_आदान_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = सुस्मृति(1, आकार(*coords));
			जर (coords == रिक्त) {
				परत NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			जर (dom_गाठ_set_user_data(
				mas->gadget.control->गाठ,
				corestring_dom_ns_किल्ली_image_coords_गाठ_data,
				coords,
				html__image_coords_dom_user_data_handler,
				&oldcoords) != DOM_NO_ERR) {
				परत NSERROR_OK;
			}
			मुक्त(oldcoords);
		}
		fallthrough;

	स्थिती GADGET_SUBMIT:
		जर (mas->gadget.control->form) {
			स्थिर अक्षर status_buffer[200];

			snprintf(status_buffer,
				 आकार status_buffer,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->result.status = status_buffer;
			mas->result.pointer = get_pointer_shape(mas->gadget.box,
								false);
			जर (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->result.action = ACTION_SUBMIT;
			}
		} अन्यथा {
			mas->result.status = messages_get("FormBadSubmit");
		}
		थांबा;

	स्थिती GADGET_TEXTBOX:
	स्थिती GADGET_PASSWORD:
	स्थिती GADGET_TEXTAREA:
		जर (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->result.status = messages_get("FormTextarea");
		} अन्यथा {
			mas->result.status = messages_get("FormTextbox");
		}

		जर (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			संघ html_selection_owner sel_owner;
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

		जर (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->result.pointer = get_pointer_shape(mas->gadget.box, false);
		} अन्यथा {
			mas->result.pointer = BROWSER_POINTER_DEFAULT;
			mas->result.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		थांबा;

	स्थिती GADGET_HIDDEN:
		/* not possible: no box generated */
		थांबा;

	स्थिती GADGET_RESET:
		mas->result.status = messages_get("FormReset");
		थांबा;

	स्थिती GADGET_FILE:
		mas->result.status = messages_get("FormFile");
		जर (mouse & BROWSER_MOUSE_CLICK_1) {
			संदेश_data.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_संदेश_GADGETCLICK,
					  &संदेश_data);
		}
		थांबा;

	स्थिती GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->result.status = messages_get("FormButton");
		थांबा;
	}

	परत NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
स्थिर nserror
iframe_mouse_action(संरचना browser_window *bw,
		    browser_mouse_state mouse,
		    पूर्णांक x, पूर्णांक y,
		    संरचना mouse_action_state *mas)
{
	पूर्णांक pos_x, pos_y;
	दशांश scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	जर (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_window_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} अन्यथा {
		browser_window_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->result.action = ACTION_NOSEND;

	परत NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
स्थिर nserror
html_object_mouse_action(html_content *html,
			 संरचना browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y,
			 संरचना mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	जर (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		संघ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	जर (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		content_mouse_action(mas->html_object.box->object,
				     bw,
				     mouse,
				     x - mas->html_object.pos_x,
				     y - mas->html_object.pos_y);
	} अन्यथा {
		content_mouse_track(mas->html_object.box->object,
				    bw,
				    mouse,
				    x - mas->html_object.pos_x,
				    y - mas->html_object.pos_y);
	}

	mas->result.action = ACTION_NOSEND;
	परत NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
स्थिर bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	जर (scheme != रिक्त) {
		जर (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	परत is_js;
}


/**
 * process mouse activity on a link
 */
स्थिर nserror
link_mouse_action(html_content *html,
		  संरचना browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y,
		  संरचना mouse_action_state *mas)
{
	nserror res;
	अक्षर *url_s = रिक्त;
	माप_प्रकार url_l = 0;
	स्थिर अक्षर status_buffer[200];
	संघ content_संदेश_data संदेश_data;

	जर (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		जर (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = रिक्त;
		}
	}

	जर (mas->title) {
		snprintf(status_buffer,
			 आकार status_buffer,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} अन्यथा {
		snprintf(status_buffer,
			 आकार status_buffer,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	जर (url_s != रिक्त) {
		मुक्त(url_s);
	}

	mas->result.status = status_buffer;

	mas->result.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	जर (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((संरचना content *)html),
					BW_NAVIGATE_DOWNLOAD,
					रिक्त,
					रिक्त,
					रिक्त);

	} अन्यथा जर (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		संदेश_data.savelink.url = mas->link.url;
		संदेश_data.savelink.title = mas->title;
		content_broadcast((संरचना content *)html,
				  CONTENT_संदेश_SAVELINK,
				  &संदेश_data);

	} अन्यथा जर (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		जर (is_javascript_navigate_url(mas->link.url)) {
			mas->result.action = ACTION_JS;
		} अन्यथा {
			mas->result.action = ACTION_NAVIGATE;
		}
	}

	परत NSERROR_OK;
}


स्थिर nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	जर (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		संघ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, रिक्त);
	}

	परत NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
स्थिर nserror
default_mouse_action(html_content *html,
		  संरचना browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y,
		  संरचना mouse_action_state *mas)
{
	संरचना content *c = (संरचना content *)html;

	/* frame resizing */
	जर (browser_window_frame_resize_start(bw, mouse, x, y, &mas->result.pointer)) {
		जर (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->result.status = messages_get("FrameDrag");
		}
		परत default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	संघ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	जर (click && html->focus_type != HTML_FOCUS_SELF) {
		संघ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, रिक्त);
	}
	जर (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	जर (mas->text.box) {
		पूर्णांक pixel_offset;
		माप_प्रकार idx;
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

		जर (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			संघ html_drag_owner drag_owner;

			जर (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   रिक्त);
				mas->result.status = messages_get("Selecting");
			}

			जर (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} अन्यथा जर (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			परत default_mouse_action_focus(html, mouse);
		}

	} अन्यथा जर (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	जर (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} अन्यथा जर (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	जर (mas->title) {
		mas->result.status = mas->title;
	}

	जर (mouse & BROWSER_MOUSE_DRAG_1) {
		जर (mouse & BROWSER_MOUSE_MOD_2) {
			संघ content_संदेश_data संदेश_data;
			संदेश_data.dragsave.type = CONTENT_SAVE_COMPLETE;
			संदेश_data.dragsave.content = रिक्त;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_data);
		} अन्यथा {
			जर (mas->drag_candidate == रिक्त) {
				browser_window_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	} अन्यथा जर (mouse & BROWSER_MOUSE_DRAG_2) {
		जर (mouse & BROWSER_MOUSE_MOD_2) {
			संघ content_संदेश_data संदेश_data;
			संदेश_data.dragsave.type = CONTENT_SAVE_SOURCE;
			संदेश_data.dragsave.content = रिक्त;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_data);
		} अन्यथा {
			जर (mas->drag_candidate == रिक्त) {
				browser_window_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->result.pointer = BROWSER_POINTER_MOVE;
		}
	}


	परत default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
स्थिर nserror
mouse_action_drag_none(html_content *html,
		       संरचना browser_window *bw,
		       browser_mouse_state mouse,
		       पूर्णांक x, पूर्णांक y)
{
	nserror res;
	संरचना content *c = (संरचना content *)html;
	संघ content_संदेश_data संदेश_data;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	स्थिर संरचना mouse_action_state mas;

	res = get_mouse_action_गाठ(html, x, y, &mas);
	जर (res != NSERROR_OK) {
		परत res;
	}

	जर (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.result.action = ACTION_BACK;
	} अन्यथा जर (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.result.action = ACTION_FORWARD;
	} अन्यथा जर (mas.scroll.bar) {
		mas.result.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.result.pointer = BROWSER_POINTER_DEFAULT;

	} अन्यथा जर (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} अन्यथा जर ((mas.object != रिक्त) && (mouse & BROWSER_MOUSE_MOD_2)) {

		जर (mouse & BROWSER_MOUSE_DRAG_2) {
			संदेश_data.dragsave.type = CONTENT_SAVE_NATIVE;
			संदेश_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_data);

		} अन्यथा जर (mouse & BROWSER_MOUSE_DRAG_1) {
			संदेश_data.dragsave.type = CONTENT_SAVE_ORIG;
			संदेश_data.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_data);
		}

		/* \todo should have a drag-saving object msg */

	} अन्यथा जर (mas.iframe != रिक्त) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} अन्यथा जर (mas.html_object.box != रिक्त) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा जर (mas.link.url != रिक्त) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	जर (res != NSERROR_OK) {
		परत res;
	}

	/* send status and pointer message */
	जर (mas.result.action != ACTION_NOSEND) {
		संदेश_data.explicit_status_text = mas.result.status;
		content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_data);

		संदेश_data.pointer = mas.result.pointer;
		content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_data);
	}

	/* fire dom click event */
	जर (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.गाठ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	निवड (mas.result.action) {
	स्थिती ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		थांबा;

	स्थिती ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				रिक्त,
				रिक्त,
				रिक्त);
		थांबा;

	स्थिती ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		जर (path != रिक्त) {
			html_exec(c,
				  lwc_string_data(path),
				  lwc_string_length(path));
			lwc_string_unref(path);
		}
		थांबा;

	स्थिती ACTION_BACK:
		res = browser_window_history_back(bw, false);
		थांबा;

	स्थिती ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		थांबा;

	स्थिती ACTION_NOSEND:
	स्थिती ACTION_NONE:
		res = NSERROR_OK;
		थांबा;
	}

	परत res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(संरचना content *c,
			 संरचना browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	परत html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(संरचना content *c,
		  संरचना browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	जर (html->visible_select_menu != रिक्त) {
		परत mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	निवड (html->drag_type) {
	स्थिती HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		थांबा;

	स्थिती HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		थांबा;

	स्थिती HTML_DRAG_TEXTAREA_SELECTION:
	स्थिती HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		थांबा;

	स्थिती HTML_DRAG_CONTENT_SELECTION:
	स्थिती HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		थांबा;

	स्थिती HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		थांबा;

	पूर्वनिर्धारित:
		/* Unknown content related drag type */
		प्रतिज्ञा(0 && "Unknown content related drag type");
	}

	जर (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	परत res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(संरचना content *c, uint32_t किल्ली)
{
	html_content *html = (html_content *) c;
	संरचना selection *sel = html->sel;

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
	जर (html->layout != रिक्त && html->layout->गाठ != रिक्त) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->गाठ, true, true, किल्ली);
	}

	निवड (html->focus_type) {
	स्थिती HTML_FOCUS_CONTENT:
		परत content_keypress(html->focus_owner.content->object, किल्ली);

	स्थिती HTML_FOCUS_TEXTAREA:
		जर (box_textarea_keypress(html, html->focus_owner.textarea, किल्ली) == NSERROR_OK) {
			परत true;
		} अन्यथा {
			परत false;
		}

	पूर्वनिर्धारित:
		/* Deal with it below */
		थांबा;
	}

	निवड (किल्ली) {
	स्थिती NS_किल्ली_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		परत true;

	स्थिती NS_किल्ली_CLEAR_SELECTION:
		selection_clear(sel, true);
		परत true;

	स्थिती NS_किल्ली_SELECT_ALL:
		selection_select_all(sel);
		परत true;

	स्थिती NS_किल्ली_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		परत selection_clear(sel, true);
	}

	परत false;
}


/**
 * Callback for in-page scrollbars.
 */
शून्य html_overflow_scroll_callback(शून्य *client_data,
		संरचना scrollbar_संदेश_data *scrollbar_data)
{
	संरचना html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	संरचना box *box = data->box;
	संघ content_संदेश_data संदेश_data;
	html_drag_type drag_type;
	संघ html_drag_owner drag_owner;

	निवड(scrollbar_data->संदेश) {
	स्थिती SCROLLBAR_संदेश_MOVED:

		जर (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			थांबा;
		}

		html__redraw_a_box(html, box);
		थांबा;
	स्थिती SCROLLBAR_संदेश_SCROLL_START:
	{
		संरचना rect rect = {
			.x0 = scrollbar_data->x0,
			.y0 = scrollbar_data->y0,
			.x1 = scrollbar_data->x1,
			.y1 = scrollbar_data->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_data->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		थांबा;
	स्थिती SCROLLBAR_संदेश_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, रिक्त);

		संदेश_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(data->c, CONTENT_संदेश_POINTER, &संदेश_data);
		थांबा;
	}
}


/* Documented in html_internal.h */
शून्य html_set_drag_type(html_content *html, html_drag_type drag_type,
		संघ html_drag_owner drag_owner, स्थिरांक संरचना rect *rect)
{
	संघ content_संदेश_data संदेश_data;

	प्रतिज्ञा(html != रिक्त);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	निवड (drag_type) {
	स्थिती HTML_DRAG_NONE:
		प्रतिज्ञा(drag_owner.no_owner == true);
		संदेश_data.drag.type = CONTENT_DRAG_NONE;
		थांबा;

	स्थिती HTML_DRAG_SCROLLBAR:
	स्थिती HTML_DRAG_TEXTAREA_SCROLLBAR:
	स्थिती HTML_DRAG_CONTENT_SCROLL:
		संदेश_data.drag.type = CONTENT_DRAG_SCROLL;
		थांबा;

	स्थिती HTML_DRAG_SELECTION:
		प्रतिज्ञा(drag_owner.no_owner == true);
		fallthrough;
	स्थिती HTML_DRAG_TEXTAREA_SELECTION:
	स्थिती HTML_DRAG_CONTENT_SELECTION:
		संदेश_data.drag.type = CONTENT_DRAG_SELECTION;
		थांबा;
	}
	संदेश_data.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((संरचना content *)html, CONTENT_संदेश_DRAG, &संदेश_data);
}

/* Documented in html_internal.h */
शून्य html_set_focus(html_content *html, html_focus_type focus_type,
		संघ html_focus_owner focus_owner, bool hide_caret,
		पूर्णांक x, पूर्णांक y, पूर्णांक height, स्थिरांक संरचना rect *clip)
{
	संघ content_संदेश_data संदेश_data;
	पूर्णांक x_off = 0;
	पूर्णांक y_off = 0;
	संरचना rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	प्रतिज्ञा(html != रिक्त);

	निवड (focus_type) {
	स्थिती HTML_FOCUS_SELF:
		प्रतिज्ञा(focus_owner.self == true);
		जर (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			परत;
		थांबा;

	स्थिती HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		थांबा;

	स्थिती HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		थांबा;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	जर (textarea_lost_focus) {
		संदेश_data.caret.type = CONTENT_CARET_REMOVE;
	} अन्यथा जर (focus_type != HTML_FOCUS_SELF && hide_caret) {
		संदेश_data.caret.type = CONTENT_CARET_HIDE;
	} अन्यथा {
		जर (clip != रिक्त) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		संदेश_data.caret.type = CONTENT_CARET_SET_POS;
		संदेश_data.caret.pos.x = x + x_off;
		संदेश_data.caret.pos.y = y + y_off;
		संदेश_data.caret.pos.height = height;
		संदेश_data.caret.pos.clip = (clip == रिक्त) ? रिक्त : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((संरचना content *)html, CONTENT_संदेश_CARET, &संदेश_data);
}

/* Documented in html_internal.h */
शून्य html_set_selection(html_content *html, html_selection_type selection_type,
		संघ html_selection_owner selection_owner, bool read_only)
{
	संघ content_संदेश_data संदेश_data;
	संरचना box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	प्रतिज्ञा(html != रिक्त);

	जर ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	जर (html->selection_type != HTML_SELECTION_NONE) {
		निवड (html->selection_type) {
		स्थिती HTML_SELECTION_SELF:
			जर (same_type)
				थांबा;
			selection_clear(html->sel, true);
			थांबा;
		स्थिती HTML_SELECTION_TEXTAREA:
			जर (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				थांबा;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->data.text.ta);
			थांबा;
		स्थिती HTML_SELECTION_CONTENT:
			जर (same_type && html->selection_owner.content ==
					selection_owner.content)
				थांबा;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			थांबा;
		पूर्वनिर्धारित:
			थांबा;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	जर (!changed)
		/* Don't need to report lack of change to owner */
		परत;

	/* Prepare msg */
	निवड (selection_type) {
	स्थिती HTML_SELECTION_NONE:
		प्रतिज्ञा(selection_owner.none == true);
		संदेश_data.selection.selection = false;
		थांबा;
	स्थिती HTML_SELECTION_SELF:
		प्रतिज्ञा(selection_owner.none == false);
		fallthrough;
	स्थिती HTML_SELECTION_TEXTAREA:
	स्थिती HTML_SELECTION_CONTENT:
		संदेश_data.selection.selection = true;
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}
	संदेश_data.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((संरचना content *)html, CONTENT_संदेश_SELECTION,
			&संदेश_data);
}
