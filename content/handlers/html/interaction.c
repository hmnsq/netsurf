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

#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <string.h>

#समावेश <dom/dom.h>

#समावेश "utils/corestrings.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/nsoption.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/mouse.h"
#समावेश "netsurf/misc.h"
#समावेश "netsurf/layout.h"
#समावेश "netsurf/keypress.h"
#समावेश "content/hlcache.h"
#समावेश "content/textsearch.h"
#समावेश "desktop/browser_history.h"
#समावेश "desktop/frames.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/selection.h"
#समावेश "desktop/textarea.h"
#समावेश "javascript/js.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/box.h"
#समावेश "html/box_textarea.h"
#समावेश "html/box_inspect.h"
#समावेश "html/font.h"
#समावेश "html/form_internal.h"
#समावेश "html/private.h"
#समावेश "html/imagemap.h"
#समावेश "html/interaction.h"

/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

अटल browser_pointer_shape get_pointer_shape(काष्ठा box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	क्रमागत css_cursor_e cursor;
	lwc_string **cursor_uris;

	अगर (box->type == BOX_FLOAT_बायाँ || box->type == BOX_FLOAT_दायाँ)
		style = box->children->style;
	अन्यथा
		style = box->style;

	अगर (style == शून्य)
		वापस BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	चयन (cursor) {
	हाल CSS_CURSOR_AUTO:
		अगर (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} अन्यथा अगर (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} अन्यथा {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		अवरोध;
	हाल CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		अवरोध;
	हाल CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		अवरोध;
	हाल CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		अवरोध;
	हाल CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_दायाँ;
		अवरोध;
	हाल CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_बायाँ;
		अवरोध;
	हाल CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		अवरोध;
	हाल CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		अवरोध;
	हाल CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		अवरोध;
	हाल CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		अवरोध;
	हाल CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		अवरोध;
	हाल CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		अवरोध;
	हाल CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		अवरोध;
	हाल CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		अवरोध;
	हाल CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		अवरोध;
	हाल CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		अवरोध;
	शेष:
		pointer = BROWSER_POINTER_DEFAULT;
		अवरोध;
	}

	वापस pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

अटल व्योम html_box_drag_start(काष्ठा box *box, पूर्णांक x, पूर्णांक y)
{
	पूर्णांक box_x, box_y;
	पूर्णांक scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	अगर (box->scroll_x != शून्य) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा अगर (box->scroll_y != शून्य) {
		scroll_mouse_x = x - (box_x + box->padding[बायाँ_2] +
				box->width + box->padding[दायाँ_2] -
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
अटल माप_प्रकार html_selection_drag_end(काष्ठा html_content *html,
		browser_mouse_state mouse, पूर्णांक x, पूर्णांक y, पूर्णांक dir)
{
	पूर्णांक pixel_offset;
	काष्ठा box *box;
	पूर्णांक dx, dy;
	माप_प्रकार idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	अगर (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->लंबाई,
				       dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	वापस idx;
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
अटल व्योम
html_image_coords_dom_user_तथ्य_handler(dom_गाँठ_operation operation,
					 dom_string *कुंजी,
					 व्योम *तथ्य_2,
					 काष्ठा dom_गाँठ *src,
					 काष्ठा dom_गाँठ *dst)
{
	काष्ठा image_निवेश_coords *oldcoords, *coords = तथ्य_2, *newcoords;

	अगर (!dom_string_isequal(corestring_dom_ns_कुंजी_image_coords_गाँठ_तथ्य,
				कुंजी) || coords == शून्य) {
		वापस;
	}

	चयन (operation) {
	हाल DOM_गाँठ_CLONED:
		newcoords = सुस्मृति(1, माप(*newcoords));
		अगर (newcoords != शून्य) {
			*newcoords = *coords;
			अगर (dom_गाँठ_set_user_तथ्य(dst,
				 corestring_dom_ns_कुंजी_image_coords_गाँठ_तथ्य,
				 newcoords,
				 html_image_coords_dom_user_तथ्य_handler,
				 &oldcoords) == DOM_NO_ERR) {
				मुक्त(oldcoords);
			}
		}
		अवरोध;

	हाल DOM_गाँठ_DELETED:
		मुक्त(coords);
		अवरोध;

	हाल DOM_गाँठ_RENAMED:
	हाल DOM_गाँठ_IMPORTED:
	हाल DOM_गाँठ_ADOPTED:
		अवरोध;

	शेष:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		निश्चित(0);
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
अटल व्योम
html_overflow_scroll_drag_end(काष्ठा scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      पूर्णांक x, पूर्णांक y)
{
	पूर्णांक scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	काष्ठा html_scrollbar_तथ्य *तथ्य = scrollbar_get_तथ्य(scrollbar);
	काष्ठा box *box;

	box = तथ्य->box;
	box_coords(box, &box_x, &box_y);

	अगर (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[बायाँ_2] +
				box->width + box->padding[दायाँ_2] -
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
अटल nserror
mouse_action_select_menu(html_content *html,
			 काष्ठा browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	काष्ठा box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;
	स्थिर अक्षर *status;
	पूर्णांक width, height;
	काष्ठा hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	निश्चित(html->visible_select_menu != शून्य);

	bw_drag_type = browser_window_get_drag_type(bw);
	अगर (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[बायाँ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
		box->padding[BOTTOM] + box->padding[शीर्ष_2];

	status = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	अगर (status != शून्य) {
		/* set status if menu still open */
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;
		संदेश_तथ्य.explicit_status_text = status;
		content_broadcast((काष्ठा content *)html,
				  CONTENT_संदेश_STATUS,
				  &संदेश_तथ्य);
		वापस NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &width, &height);

	html->visible_select_menu = शून्य;

	bw_content = browser_window_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       width,
			       height);
	वापस NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
अटल nserror
mouse_action_drag_selection(html_content *html,
			    काष्ठा browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	काष्ठा box *box;
	पूर्णांक dir = -1;
	पूर्णांक dx, dy;
	माप_प्रकार idx;
	जोड़ html_drag_owner drag_owner;
	पूर्णांक pixel_offset;
	plot_font_style_t fstyle;

	अगर (!mouse) {
		/* End of selection drag */
		अगर (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		अगर (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_set_drag_type(html, HTML_DRAG_NONE, drag_owner, शून्य);

		वापस NSERROR_OK;
	}

	अगर (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	अगर (box != शून्य) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->लंबाई,
				       dx,
				       &idx,
				       &pixel_offset);

		selection_track(html->sel, mouse, box->byte_offset + idx);
	}
	वापस NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
अटल nserror
mouse_action_drag_scrollbar(html_content *html,
			    काष्ठा browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	काष्ठा scrollbar *scr;
	काष्ठा html_scrollbar_तथ्य *तथ्य;
	काष्ठा box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;
	स्थिर अक्षर *status;
	पूर्णांक scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_status scrollbar_status;

	scr = html->drag_owner.scrollbar;

	अगर (!mouse) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(scr, mouse, x, y);
	}

	तथ्य = scrollbar_get_तथ्य(scr);

	box = तथ्य->box;

	box_coords(box, &box_x, &box_y);

	अगर (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शीर्ष_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_WIDTH);
		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[बायाँ_2] +
				      box->width + box->padding[दायाँ_2] -
				      SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_status = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	status = scrollbar_mouse_status_to_message(scrollbar_status);

	अगर (status != शून्य) {
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;

		संदेश_तथ्य.explicit_status_text = status;
		content_broadcast((काष्ठा content *)html,
				  CONTENT_संदेश_STATUS,
				  &संदेश_तथ्य);
	}

	वापस NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
अटल nserror
mouse_action_drag_textarea(html_content *html,
			    काष्ठा browser_window *bw,
			    browser_mouse_state mouse,
			    पूर्णांक x, पूर्णांक y)
{
	काष्ठा box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	box = html->drag_owner.textarea;

	निश्चित(box->gadget != शून्य);
	निश्चित(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->तथ्य.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	वापस NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
अटल nserror
mouse_action_drag_content(html_content *html,
			  काष्ठा browser_window *bw,
			  browser_mouse_state mouse,
			  पूर्णांक x, पूर्णांक y)
{
	काष्ठा box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	box = html->drag_owner.content;
	निश्चित(box->object != शून्य);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	वापस NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
काष्ठा mouse_action_state {
	काष्ठा {
		स्थिर अक्षर *status; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		क्रमागत {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_BACK, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} परिणाम;

	/** dom node */
	काष्ठा dom_गाँठ *गाँठ;

	/** html object */
	काष्ठा {
		काष्ठा box *box;
		पूर्णांक pos_x;
		पूर्णांक pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	काष्ठा browser_window *iframe;

	/** link either from href or imagemap */
	काष्ठा {
		काष्ठा box *box;
		nsurl *url;
		स्थिर अक्षर *target;
		bool is_imagemap;
	} link;

	/** gadget */
	काष्ठा {
		काष्ठा form_control *control;
		काष्ठा box *box;
		पूर्णांक box_x;
		पूर्णांक box_y;
		स्थिर अक्षर *target;
	} gadget;

	/** title */
	स्थिर अक्षर *title;

	/** candidate box for drag operation */
	काष्ठा box *drag_candidate;

	/** scrollbar */
	काष्ठा {
		काष्ठा scrollbar *bar;
		पूर्णांक mouse_x;
		पूर्णांक mouse_y;
	} scroll;

	/** text in box */
	काष्ठा {
		काष्ठा box *box;
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
अटल nserror
get_mouse_action_गाँठ(html_content *html,
		      पूर्णांक x, पूर्णांक y,
		      काष्ठा mouse_action_state *man)
{
	काष्ठा box *box;
	पूर्णांक box_x = 0;
	पूर्णांक box_y = 0;

	/* initialise the mouse action state data */
	स_रखो(man, 0, माप(काष्ठा mouse_action_state));
	man->गाँठ = html->layout->गाँठ; /* Default dom node to the <HTML> */
	man->परिणाम.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[बायाँ_2];
	box_y = box->margin[शीर्ष_2];

	करो {
		/* skip hidden boxes */
		अगर ((box->style != शून्य) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			जाओ अग्र_box;
		}

		अगर (box->गाँठ != शून्य) {
			man->गाँठ = box->गाँठ;
		}

		अगर (box->object) {
			अगर (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} अन्यथा {
				man->object = box->object;
			}
		}

		अगर (box->iframe) {
			man->iframe = box->iframe;
		}

		अगर (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		अगर (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		अगर (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			अगर (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		अगर (box->title) {
			man->title = box->title;
		}

		man->परिणाम.pointer = get_pointer_shape(box, false);

		अगर ((box->scroll_x != शून्य) ||
		    (box->scroll_y != शून्य)) {
			पूर्णांक padding_बायाँ;
			पूर्णांक padding_दायाँ;
			पूर्णांक padding_शीर्ष;
			पूर्णांक padding_bottom;

			अगर (man->drag_candidate == शून्य) {
				man->drag_candidate = box;
			}

			padding_बायाँ = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_दायाँ = padding_बायाँ + box->padding[बायाँ_2] +
					box->width + box->padding[दायाँ_2];
			padding_शीर्ष = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_शीर्ष + box->padding[शीर्ष_2] +
					box->height + box->padding[BOTTOM];

			अगर ((x > padding_बायाँ) &&
			    (x < padding_दायाँ) &&
			    (y > padding_शीर्ष) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				अगर ((box->scroll_y != शून्य) &&
				    (x > (padding_दायाँ - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_दायाँ - SCROLLBAR_WIDTH);
					man->scroll.mouse_y = y - padding_शीर्ष;
					अवरोध;

				} अन्यथा अगर ((box->scroll_x != शून्य) &&
					   (y > (padding_bottom -
							SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_बायाँ;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_WIDTH);
					अवरोध;
				}
			}
		}

		अगर (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	अग्र_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} जबतक (box != शून्य);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	निश्चित(man->गाँठ != शून्य);

	वापस NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
अटल nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    पूर्णांक x, पूर्णांक y,
		    काष्ठा mouse_action_state *mas)
{
	काष्ठा content *c = (काष्ठा content *)html;
	textarea_mouse_status ta_status;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	चयन (mas->gadget.control->type) {
	हाल GADGET_SELECT:
		mas->परिणाम.status = messages_get("FormSelect");
		mas->परिणाम.pointer = BROWSER_POINTER_MENU;
		अगर (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			अगर (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = शून्य;
			}
			mas->परिणाम.pointer = BROWSER_POINTER_DEFAULT;
		} अन्यथा अगर (mouse & BROWSER_MOUSE_CLICK_1) {
			संदेश_तथ्य.select_menu.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_संदेश_SELECTMENU,
					  &संदेश_तथ्य);
		}
		अवरोध;

	हाल GADGET_CHECKBOX:
		mas->परिणाम.status = messages_get("FormCheckbox");
		अगर (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_निवेश_element_set_checked(
				(dom_html_निवेश_element *)(mas->gadget.control->गाँठ),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		अवरोध;

	हाल GADGET_RADIO:
		mas->परिणाम.status = messages_get("FormRadio");
		अगर (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_set(mas->gadget.control);
		}
		अवरोध;

	हाल GADGET_IMAGE:
		/* This falls through to SUBMIT */
		अगर (mouse & BROWSER_MOUSE_CLICK_1) {
			काष्ठा image_निवेश_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = सुस्मृति(1, माप(*coords));
			अगर (coords == शून्य) {
				वापस NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			अगर (dom_गाँठ_set_user_तथ्य(
				mas->gadget.control->गाँठ,
				corestring_dom_ns_कुंजी_image_coords_गाँठ_तथ्य,
				coords,
				html_image_coords_dom_user_तथ्य_handler,
				&oldcoords) != DOM_NO_ERR) {
				वापस NSERROR_OK;
			}
			मुक्त(oldcoords);
		}
		fallthrough;

	हाल GADGET_SUBMIT:
		अगर (mas->gadget.control->form) {
			अटल अक्षर status_बफर[200];

			snprintf(status_बफर,
				 माप status_बफर,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->परिणाम.status = status_बफर;
			mas->परिणाम.pointer = get_pointer_shape(mas->gadget.box,
								false);
			अगर (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->परिणाम.action = ACTION_SUBMIT;
			}
		} अन्यथा {
			mas->परिणाम.status = messages_get("FormBadSubmit");
		}
		अवरोध;

	हाल GADGET_TEXTBOX:
	हाल GADGET_PASSWORD:
	हाल GADGET_TEXTAREA:
		अगर (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->परिणाम.status = messages_get("FormTextarea");
		} अन्यथा {
			mas->परिणाम.status = messages_get("FormTextbox");
		}

		अगर (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			जोड़ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_set_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_status = textarea_mouse_action(mas->gadget.control->तथ्य.text.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		अगर (ta_status & TEXTAREA_MOUSE_EDITOR) {
			mas->परिणाम.pointer = get_pointer_shape(mas->gadget.box, false);
		} अन्यथा {
			mas->परिणाम.pointer = BROWSER_POINTER_DEFAULT;
			mas->परिणाम.status = scrollbar_mouse_status_to_message(ta_status >> 3);
		}
		अवरोध;

	हाल GADGET_HIDDEN:
		/* not possible: no box generated */
		अवरोध;

	हाल GADGET_RESET:
		mas->परिणाम.status = messages_get("FormReset");
		अवरोध;

	हाल GADGET_FILE:
		mas->परिणाम.status = messages_get("FormFile");
		अगर (mouse & BROWSER_MOUSE_CLICK_1) {
			संदेश_तथ्य.gadget_click.gadget = mas->gadget.control;
			content_broadcast(c,
					  CONTENT_संदेश_GADGETCLICK,
					  &संदेश_तथ्य);
		}
		अवरोध;

	हाल GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->परिणाम.status = messages_get("FormButton");
		अवरोध;
	}

	वापस NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
अटल nserror
iframe_mouse_action(काष्ठा browser_window *bw,
		    browser_mouse_state mouse,
		    पूर्णांक x, पूर्णांक y,
		    काष्ठा mouse_action_state *mas)
{
	पूर्णांक pos_x, pos_y;
	भग्न scale;

	scale = browser_window_get_scale(bw);

	browser_window_get_position(mas->iframe, false, &pos_x, &pos_y);

	अगर (mouse & BROWSER_MOUSE_CLICK_1 ||
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
	mas->परिणाम.action = ACTION_NOSEND;

	वापस NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
अटल nserror
html_object_mouse_action(html_content *html,
			 काष्ठा browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y,
			 काष्ठा mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	अगर (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		जोड़ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	अगर (mouse & BROWSER_MOUSE_CLICK_1 ||
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

	mas->परिणाम.action = ACTION_NOSEND;
	वापस NSERROR_OK;
}


/**
 * determine if a url has a javascript scheme
 *
 * \param urm The url to check.
 * \return true if the url is a javascript scheme else false
 */
अटल bool is_javascript_navigate_url(nsurl *url)
{
	bool is_js = false;
	lwc_string *scheme;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (scheme != शून्य) {
		अगर (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	वापस is_js;
}


/**
 * process mouse activity on a link
 */
अटल nserror
link_mouse_action(html_content *html,
		  काष्ठा browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y,
		  काष्ठा mouse_action_state *mas)
{
	nserror res;
	अक्षर *url_s = शून्य;
	माप_प्रकार url_l = 0;
	अटल अक्षर status_बफर[200];
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;

	अगर (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		अगर (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = शून्य;
		}
	}

	अगर (mas->title) {
		snprintf(status_बफर,
			 माप status_बफर,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} अन्यथा {
		snprintf(status_बफर,
			 माप status_बफर,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	अगर (url_s != शून्य) {
		मुक्त(url_s);
	}

	mas->परिणाम.status = status_बफर;

	mas->परिणाम.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	अगर (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_window_navigate(bw,
					mas->link.url,
					content_get_url((काष्ठा content *)html),
					BW_NAVIGATE_DOWNLOAD,
					शून्य,
					शून्य,
					शून्य);

	} अन्यथा अगर (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		संदेश_तथ्य.savelink.url = mas->link.url;
		संदेश_तथ्य.savelink.title = mas->title;
		content_broadcast((काष्ठा content *)html,
				  CONTENT_संदेश_SAVELINK,
				  &संदेश_तथ्य);

	} अन्यथा अगर (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		अगर (is_javascript_navigate_url(mas->link.url)) {
			mas->परिणाम.action = ACTION_JS;
		} अन्यथा {
			mas->परिणाम.action = ACTION_NAVIGATE;
		}
	}

	वापस NSERROR_OK;
}


अटल nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	अगर (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		जोड़ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, शून्य);
	}

	वापस NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
अटल nserror
default_mouse_action(html_content *html,
		  काष्ठा browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y,
		  काष्ठा mouse_action_state *mas)
{
	काष्ठा content *c = (काष्ठा content *)html;

	/* frame resizing */
	अगर (browser_window_frame_resize_start(bw, mouse, x, y, &mas->परिणाम.pointer)) {
		अगर (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->परिणाम.status = messages_get("FrameDrag");
		}
		वापस default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	जोड़ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	अगर (click && html->focus_type != HTML_FOCUS_SELF) {
		जोड़ html_focus_owner fo;
		fo.self = true;
		html_set_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, शून्य);
	}
	अगर (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	अगर (mas->text.box) {
		पूर्णांक pixel_offset;
		माप_प्रकार idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->text.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->text.box->text,
				       mas->text.box->लंबाई,
				       x - mas->text.box_x,
				       &idx,
				       &pixel_offset);

		अगर (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_offset + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			जोड़ html_drag_owner drag_owner;

			अगर (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_set_drag_type(html,
						   drag_type,
						   drag_owner,
						   शून्य);
				mas->परिणाम.status = messages_get("Selecting");
			}

			अगर (selection_active(html->sel)) {
				sel_owner.none = false;
				html_set_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} अन्यथा अगर (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_set_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			वापस default_mouse_action_focus(html, mouse);
		}

	} अन्यथा अगर (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	अगर (selection_active(html->sel)) {
		sel_owner.none = false;
		html_set_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} अन्यथा अगर (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_set_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	अगर (mas->title) {
		mas->परिणाम.status = mas->title;
	}

	अगर (mouse & BROWSER_MOUSE_DRAG_1) {
		अगर (mouse & BROWSER_MOUSE_MOD_2) {
			जोड़ content_संदेश_तथ्य संदेश_तथ्य;
			संदेश_तथ्य.dragsave.type = CONTENT_SAVE_COMPLETE;
			संदेश_तथ्य.dragsave.content = शून्य;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_तथ्य);
		} अन्यथा {
			अगर (mas->drag_candidate == शून्य) {
				browser_window_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->परिणाम.pointer = BROWSER_POINTER_MOVE;
		}
	} अन्यथा अगर (mouse & BROWSER_MOUSE_DRAG_2) {
		अगर (mouse & BROWSER_MOUSE_MOD_2) {
			जोड़ content_संदेश_तथ्य संदेश_तथ्य;
			संदेश_तथ्य.dragsave.type = CONTENT_SAVE_SOURCE;
			संदेश_तथ्य.dragsave.content = शून्य;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_तथ्य);
		} अन्यथा {
			अगर (mas->drag_candidate == शून्य) {
				browser_window_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->परिणाम.pointer = BROWSER_POINTER_MOVE;
		}
	}


	वापस default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
अटल nserror
mouse_action_drag_none(html_content *html,
		       काष्ठा browser_window *bw,
		       browser_mouse_state mouse,
		       पूर्णांक x, पूर्णांक y)
{
	nserror res;
	काष्ठा content *c = (काष्ठा content *)html;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	lwc_string *path;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	अटल काष्ठा mouse_action_state mas;

	res = get_mouse_action_गाँठ(html, x, y, &mas);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	अगर (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.परिणाम.action = ACTION_BACK;
	} अन्यथा अगर (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.परिणाम.action = ACTION_FORWARD;
	} अन्यथा अगर (mas.scroll.bar) {
		mas.परिणाम.status = scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.परिणाम.pointer = BROWSER_POINTER_DEFAULT;

	} अन्यथा अगर (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} अन्यथा अगर ((mas.object != शून्य) && (mouse & BROWSER_MOUSE_MOD_2)) {

		अगर (mouse & BROWSER_MOUSE_DRAG_2) {
			संदेश_तथ्य.dragsave.type = CONTENT_SAVE_NATIVE;
			संदेश_तथ्य.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_तथ्य);

		} अन्यथा अगर (mouse & BROWSER_MOUSE_DRAG_1) {
			संदेश_तथ्य.dragsave.type = CONTENT_SAVE_ORIG;
			संदेश_तथ्य.dragsave.content = mas.object;
			content_broadcast(c, CONTENT_संदेश_DRAGSAVE, &संदेश_तथ्य);
		}

		/* \todo should have a drag-saving object msg */

	} अन्यथा अगर (mas.iframe != शून्य) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} अन्यथा अगर (mas.html_object.box != शून्य) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा अगर (mas.link.url != शून्य) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	/* send status and pointer message */
	अगर (mas.परिणाम.action != ACTION_NOSEND) {
		संदेश_तथ्य.explicit_status_text = mas.परिणाम.status;
		content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_तथ्य);

		संदेश_तथ्य.pointer = mas.परिणाम.pointer;
		content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_तथ्य);
	}

	/* fire dom click event */
	अगर (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.गाँठ, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	चयन (mas.परिणाम.action) {
	हाल ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_window_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		अवरोध;

	हाल ACTION_NAVIGATE:
		res = browser_window_navigate(
				browser_window_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				शून्य,
				शून्य,
				शून्य);
		अवरोध;

	हाल ACTION_JS:
		path = nsurl_get_component(mas.link.url, NSURL_PATH);
		अगर (path != शून्य) {
			html_exec(c,
				  lwc_string_तथ्य(path),
				  lwc_string_लंबाई(path));
			lwc_string_unref(path);
		}
		अवरोध;

	हाल ACTION_BACK:
		res = browser_window_history_back(bw, false);
		अवरोध;

	हाल ACTION_FORWARD:
		res = browser_window_history_forward(bw, false);
		अवरोध;

	हाल ACTION_NOSEND:
	हाल ACTION_NONE:
		res = NSERROR_OK;
		अवरोध;
	}

	वापस res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(काष्ठा content *c,
			 काष्ठा browser_window *bw,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	वापस html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(काष्ठा content *c,
		  काष्ठा browser_window *bw,
		  browser_mouse_state mouse,
		  पूर्णांक x, पूर्णांक y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	अगर (html->visible_select_menu != शून्य) {
		वापस mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	चयन (html->drag_type) {
	हाल HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		अवरोध;

	हाल HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		अवरोध;

	हाल HTML_DRAG_TEXTAREA_SELECTION:
	हाल HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		अवरोध;

	हाल HTML_DRAG_CONTENT_SELECTION:
	हाल HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		अवरोध;

	हाल HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		अवरोध;

	शेष:
		/* Unknown content related drag type */
		निश्चित(0 && "Unknown content related drag type");
	}

	अगर (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	वापस res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(काष्ठा content *c, uint32_t कुंजी)
{
	html_content *html = (html_content *) c;
	काष्ठा selection *sel = html->sel;

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
	अगर (html->layout != शून्य && html->layout->गाँठ != शून्य) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->गाँठ, true, true, कुंजी);
	}

	चयन (html->focus_type) {
	हाल HTML_FOCUS_CONTENT:
		वापस content_keypress(html->focus_owner.content->object, कुंजी);

	हाल HTML_FOCUS_TEXTAREA:
		अगर (box_textarea_keypress(html, html->focus_owner.textarea, कुंजी) == NSERROR_OK) {
			वापस true;
		} अन्यथा {
			वापस false;
		}

	शेष:
		/* Deal with it below */
		अवरोध;
	}

	चयन (कुंजी) {
	हाल NS_कुंजी_COPY_SELECTION:
		selection_copy_to_clipboard(sel);
		वापस true;

	हाल NS_कुंजी_CLEAR_SELECTION:
		selection_clear(sel, true);
		वापस true;

	हाल NS_कुंजी_SELECT_ALL:
		selection_select_all(sel);
		वापस true;

	हाल NS_कुंजी_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		वापस selection_clear(sel, true);
	}

	वापस false;
}


/**
 * Callback for in-page scrollbars.
 */
व्योम html_overflow_scroll_callback(व्योम *client_तथ्य,
		काष्ठा scrollbar_संदेश_तथ्य *scrollbar_तथ्य)
{
	काष्ठा html_scrollbar_तथ्य *तथ्य = client_तथ्य;
	html_content *html = (html_content *)तथ्य->c;
	काष्ठा box *box = तथ्य->box;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	html_drag_type drag_type;
	जोड़ html_drag_owner drag_owner;

	चयन(scrollbar_तथ्य->संदेश) {
	हाल SCROLLBAR_संदेश_MOVED:

		अगर (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			अवरोध;
		}

		html__redraw_a_box(html, box);
		अवरोध;
	हाल SCROLLBAR_संदेश_SCROLL_START:
	{
		काष्ठा rect rect = {
			.x0 = scrollbar_तथ्य->x0,
			.y0 = scrollbar_तथ्य->y0,
			.x1 = scrollbar_तथ्य->x1,
			.y1 = scrollbar_तथ्य->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_तथ्य->scrollbar;
		html_set_drag_type(html, drag_type, drag_owner, &rect);
	}
		अवरोध;
	हाल SCROLLBAR_संदेश_SCROLL_FINISHED:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_set_drag_type(html, drag_type, drag_owner, शून्य);

		संदेश_तथ्य.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(तथ्य->c, CONTENT_संदेश_POINTER, &संदेश_तथ्य);
		अवरोध;
	}
}


/* Documented in html_internal.h */
व्योम html_set_drag_type(html_content *html, html_drag_type drag_type,
		जोड़ html_drag_owner drag_owner, स्थिर काष्ठा rect *rect)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;

	निश्चित(html != शून्य);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	चयन (drag_type) {
	हाल HTML_DRAG_NONE:
		निश्चित(drag_owner.no_owner == true);
		संदेश_तथ्य.drag.type = CONTENT_DRAG_NONE;
		अवरोध;

	हाल HTML_DRAG_SCROLLBAR:
	हाल HTML_DRAG_TEXTAREA_SCROLLBAR:
	हाल HTML_DRAG_CONTENT_SCROLL:
		संदेश_तथ्य.drag.type = CONTENT_DRAG_SCROLL;
		अवरोध;

	हाल HTML_DRAG_SELECTION:
		निश्चित(drag_owner.no_owner == true);
		fallthrough;
	हाल HTML_DRAG_TEXTAREA_SELECTION:
	हाल HTML_DRAG_CONTENT_SELECTION:
		संदेश_तथ्य.drag.type = CONTENT_DRAG_SELECTION;
		अवरोध;
	}
	संदेश_तथ्य.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_broadcast((काष्ठा content *)html, CONTENT_संदेश_DRAG, &संदेश_तथ्य);
}

/* Documented in html_internal.h */
व्योम html_set_focus(html_content *html, html_focus_type focus_type,
		जोड़ html_focus_owner focus_owner, bool hide_caret,
		पूर्णांक x, पूर्णांक y, पूर्णांक height, स्थिर काष्ठा rect *clip)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	पूर्णांक x_off = 0;
	पूर्णांक y_off = 0;
	काष्ठा rect cr;
	bool textarea_lost_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	निश्चित(html != शून्य);

	चयन (focus_type) {
	हाल HTML_FOCUS_SELF:
		निश्चित(focus_owner.self == true);
		अगर (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			वापस;
		अवरोध;

	हाल HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		अवरोध;

	हाल HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		अवरोध;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	अगर (textarea_lost_focus) {
		संदेश_तथ्य.caret.type = CONTENT_CARET_REMOVE;
	} अन्यथा अगर (focus_type != HTML_FOCUS_SELF && hide_caret) {
		संदेश_तथ्य.caret.type = CONTENT_CARET_HIDE;
	} अन्यथा {
		अगर (clip != शून्य) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		संदेश_तथ्य.caret.type = CONTENT_CARET_SET_POS;
		संदेश_तथ्य.caret.pos.x = x + x_off;
		संदेश_तथ्य.caret.pos.y = y + y_off;
		संदेश_तथ्य.caret.pos.height = height;
		संदेश_तथ्य.caret.pos.clip = (clip == शून्य) ? शून्य : &cr;
	}

	/* Inform of the content's drag status change */
	content_broadcast((काष्ठा content *)html, CONTENT_संदेश_CARET, &संदेश_तथ्य);
}

/* Documented in html_internal.h */
व्योम html_set_selection(html_content *html, html_selection_type selection_type,
		जोड़ html_selection_owner selection_owner, bool read_only)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	काष्ठा box *box;
	bool changed = false;
	bool same_type = html->selection_type == selection_type;

	निश्चित(html != शून्य);

	अगर ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		changed = true;

	/* Clear any existing selection */
	अगर (html->selection_type != HTML_SELECTION_NONE) {
		चयन (html->selection_type) {
		हाल HTML_SELECTION_SELF:
			अगर (same_type)
				अवरोध;
			selection_clear(html->sel, true);
			अवरोध;
		हाल HTML_SELECTION_TEXTAREA:
			अगर (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				अवरोध;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->तथ्य.text.ta);
			अवरोध;
		हाल HTML_SELECTION_CONTENT:
			अगर (same_type && html->selection_owner.content ==
					selection_owner.content)
				अवरोध;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			अवरोध;
		शेष:
			अवरोध;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	अगर (!changed)
		/* Don't need to report lack of change to owner */
		वापस;

	/* Prepare msg */
	चयन (selection_type) {
	हाल HTML_SELECTION_NONE:
		निश्चित(selection_owner.none == true);
		संदेश_तथ्य.selection.selection = false;
		अवरोध;
	हाल HTML_SELECTION_SELF:
		निश्चित(selection_owner.none == false);
		fallthrough;
	हाल HTML_SELECTION_TEXTAREA:
	हाल HTML_SELECTION_CONTENT:
		संदेश_तथ्य.selection.selection = true;
		अवरोध;
	शेष:
		अवरोध;
	}
	संदेश_तथ्य.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_broadcast((काष्ठा content *)html, CONTENT_संदेश_SELECTION,
			&संदेश_तथ्य);
}
