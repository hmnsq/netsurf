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

	यदि (box->type == BOX_FLOAT_वामः || box->type == BOX_FLOAT_दक्षिणः)
		style = box->children->style;
	अन्यथा
		style = box->style;

	यदि (style == रिक्त)
		प्रत्यागम BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	विकल्प (cursor) {
	पक्ष CSS_CURSOR_AUTO:
		यदि (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} अन्यथा यदि (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} अन्यथा {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		भङ्ग;
	पक्ष CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		भङ्ग;
	पक्ष CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		भङ्ग;
	पक्ष CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		भङ्ग;
	पक्ष CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_दक्षिणः;
		भङ्ग;
	पक्ष CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_वामः;
		भङ्ग;
	पक्ष CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_ऊर्ध्वम्;
		भङ्ग;
	पक्ष CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_अधस्तात्;
		भङ्ग;
	पक्ष CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		भङ्ग;
	पक्ष CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		भङ्ग;
	पक्ष CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		भङ्ग;
	पक्ष CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		भङ्ग;
	पक्ष CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		भङ्ग;
	पक्ष CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		भङ्ग;
	पक्ष CSS_CURSOR_प्रगतिः:
		pointer = BROWSER_POINTER_प्रगतिः;
		भङ्ग;
	पक्ष CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		भङ्ग;
	स्वभाव:
		pointer = BROWSER_POINTER_DEFAULT;
		भङ्ग;
	}

	प्रत्यागम pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

स्थिर शून्य html_box_drag_start(संरचना box *box, पूर्णाङ्क x, पूर्णाङ्क y)
{
	पूर्णाङ्क box_x, box_y;
	पूर्णाङ्क scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	यदि (box->scroll_x != रिक्त) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शिखरम्_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_विस्तारः);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा यदि (box->scroll_y != रिक्त) {
		scroll_mouse_x = x - (box_x + box->padding[वामः_2] +
				box->विस्तारः + box->padding[दक्षिणः_2] -
				SCROLLBAR_विस्तारः);
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
स्थिर size_t html_selection_drag_अन्तः(संरचना html_content *html,
		browser_mouse_state mouse, पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क dir)
{
	पूर्णाङ्क pixel_विस्थापनम्;
	संरचना box *box;
	पूर्णाङ्क dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	यदि (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle, box->text, box->दीर्घता,
				       dx, &idx, &pixel_विस्थापनम्);

		idx += box->byte_विस्थापनम्;
	}

	प्रत्यागम idx;
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
html_image_coords_dom_user_दत्त_handler(dom_ग्रन्थिः_operation operation,
					 dom_string *कुञ्चिका,
					 शून्य *दत्त_2,
					 संरचना dom_ग्रन्थिः *src,
					 संरचना dom_ग्रन्थिः *dst)
{
	संरचना image_निवेशः_coords *oldcoords, *coords = दत्त_2, *newcoords;

	यदि (!dom_string_isequal(corestring_dom_ns_कुञ्चिका_image_coords_ग्रन्थिः_दत्त,
				कुञ्चिका) || coords == रिक्त) {
		प्रत्यागम;
	}

	विकल्प (operation) {
	पक्ष DOM_ग्रन्थिः_CLONED:
		newcoords = शून्यस्मृतिं_देहि(1, परिमाण(*newcoords));
		यदि (newcoords != रिक्त) {
			*newcoords = *coords;
			यदि (dom_ग्रन्थिः_समुच्चयः_user_दत्त(dst,
				 corestring_dom_ns_कुञ्चिका_image_coords_ग्रन्थिः_दत्त,
				 newcoords,
				 html_image_coords_dom_user_दत्त_handler,
				 &oldcoords) == DOM_NO_ERR) {
				मुक्त(oldcoords);
			}
		}
		भङ्ग;

	पक्ष DOM_ग्रन्थिः_DELETED:
		मुक्त(coords);
		भङ्ग;

	पक्ष DOM_ग्रन्थिः_RENAMED:
	पक्ष DOM_ग्रन्थिः_IMPORTED:
	पक्ष DOM_ग्रन्थिः_ADOPTED:
		भङ्ग;

	स्वभाव:
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
html_अतिप्रवाहः_scroll_drag_अन्तः(संरचना scrollbar *scrollbar,
			      browser_mouse_state mouse,
			      पूर्णाङ्क x, पूर्णाङ्क y)
{
	पूर्णाङ्क scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	संरचना html_scrollbar_दत्त *दत्त = scrollbar_get_दत्त(scrollbar);
	संरचना box *box;

	box = दत्त->box;
	box_coords(box, &box_x, &box_y);

	यदि (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[शिखरम्_2] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_विस्तारः);
		scrollbar_mouse_drag_अन्तः(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[वामः_2] +
				box->विस्तारः + box->padding[दक्षिणः_2] -
				SCROLLBAR_विस्तारः);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_अन्तः(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * handle html mouse action when select menu is open
 *
 */
स्थिर nserror
mouse_action_select_menu(html_content *html,
			 संरचना browser_गवाक्षः *bw,
			 browser_mouse_state mouse,
			 पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना box *box;
	पूर्णाङ्क box_x = 0;
	पूर्णाङ्क box_y = 0;
	ध्रुव वर्ण *स्थितिः;
	पूर्णाङ्क विस्तारः, height;
	संरचना hlcache_handle *bw_content;
	browser_drag_type bw_drag_type;

	प्रतिज्ञा(html->visible_select_menu != रिक्त);

	bw_drag_type = browser_गवाक्षः_get_drag_type(bw);
	यदि (bw_drag_type != DRAGGING_NONE && !mouse) {
		/* drag end: select menu */
		form_select_mouse_drag_अन्तः(html->visible_select_menu, mouse, x, y);
	}

	box = html->visible_select_menu->box;
	box_coords(box, &box_x, &box_y);

	box_x -= box->border[वामः_2].विस्तारः;
	box_y += box->height + box->border[BOTTOM].विस्तारः +
		box->padding[BOTTOM] + box->padding[शिखरम्_2];

	स्थितिः = form_select_mouse_action(html->visible_select_menu,
					  mouse,
					  x - box_x,
					  y - box_y);
	यदि (स्थितिः != रिक्त) {
		/* set status if menu still open */
		सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
		सन्देशः_दत्त.explicit_स्थितिः_text = स्थितिः;
		content_प्रसारणम्((संरचना content *)html,
				  CONTENT_सन्देशः_स्थितिः,
				  &सन्देशः_दत्त);
		प्रत्यागम NSERROR_OK;
	}

	/* close menu and redraw where it was */
	form_select_get_dimensions(html->visible_select_menu, &विस्तारः, &height);

	html->visible_select_menu = रिक्त;

	bw_content = browser_गवाक्षः_get_content(bw);
	content_request_redraw(bw_content,
			       box_x,
			       box_y,
			       विस्तारः,
			       height);
	प्रत्यागम NSERROR_OK;
}


/**
 * handle html mouse action when a selection drag is being performed
 *
 */
स्थिर nserror
mouse_action_drag_selection(html_content *html,
			    संरचना browser_गवाक्षः *bw,
			    browser_mouse_state mouse,
			    पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना box *box;
	पूर्णाङ्क dir = -1;
	पूर्णाङ्क dx, dy;
	size_t idx;
	सङ्घ html_drag_owner drag_owner;
	पूर्णाङ्क pixel_विस्थापनम्;
	plot_font_style_t fstyle;

	यदि (!mouse) {
		/* End of selection drag */
		यदि (selection_dragging_start(html->sel)) {
			dir = 1;
		}

		idx = html_selection_drag_अन्तः(html, mouse, x, y, dir);

		यदि (idx != 0) {
			selection_track(html->sel, mouse, idx);
		}

		drag_owner.no_owner = true;
		html_समुच्चयः_drag_type(html, HTML_DRAG_NONE, drag_owner, रिक्त);

		प्रत्यागम NSERROR_OK;
	}

	यदि (selection_dragging_start(html->sel)) {
		dir = 1;
	}

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	यदि (box != रिक्त) {
		font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);

		guit->layout->position(&fstyle,
				       box->text,
				       box->दीर्घता,
				       dx,
				       &idx,
				       &pixel_विस्थापनम्);

		selection_track(html->sel, mouse, box->byte_विस्थापनम् + idx);
	}
	प्रत्यागम NSERROR_OK;
}


/**
 * handle html mouse action when a scrollbar drag is being performed
 *
 */
स्थिर nserror
mouse_action_drag_scrollbar(html_content *html,
			    संरचना browser_गवाक्षः *bw,
			    browser_mouse_state mouse,
			    पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना scrollbar *scr;
	संरचना html_scrollbar_दत्त *दत्त;
	संरचना box *box;
	पूर्णाङ्क box_x = 0;
	पूर्णाङ्क box_y = 0;
	ध्रुव वर्ण *स्थितिः;
	पूर्णाङ्क scroll_mouse_x = 0, scroll_mouse_y = 0;
	scrollbar_mouse_स्थितिः scrollbar_स्थितिः;

	scr = html->drag_owner.scrollbar;

	यदि (!mouse) {
		/* drag end: scrollbar */
		html_अतिप्रवाहः_scroll_drag_अन्तः(scr, mouse, x, y);
	}

	दत्त = scrollbar_get_दत्त(scr);

	box = दत्त->box;

	box_coords(box, &box_x, &box_y);

	यदि (scrollbar_is_horizontal(scr)) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[शिखरम्_2] +
				      box->height + box->padding[BOTTOM] -
				      SCROLLBAR_विस्तारः);
		scrollbar_स्थितिः = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	} अन्यथा {
		scroll_mouse_x = x - (box_x + box->padding[वामः_2] +
				      box->विस्तारः + box->padding[दक्षिणः_2] -
				      SCROLLBAR_विस्तारः);
		scroll_mouse_y = y - box_y;

		scrollbar_स्थितिः = scrollbar_mouse_action(scr,
							  mouse,
							  scroll_mouse_x,
							  scroll_mouse_y);
	}
	स्थितिः = scrollbar_mouse_स्थितिः_गन्तव्यम्_सन्देशः(scrollbar_स्थितिः);

	यदि (स्थितिः != रिक्त) {
		सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;

		सन्देशः_दत्त.explicit_स्थितिः_text = स्थितिः;
		content_प्रसारणम्((संरचना content *)html,
				  CONTENT_सन्देशः_स्थितिः,
				  &सन्देशः_दत्त);
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a text area
 */
स्थिर nserror
mouse_action_drag_textarea(html_content *html,
			    संरचना browser_गवाक्षः *bw,
			    browser_mouse_state mouse,
			    पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना box *box;
	पूर्णाङ्क box_x = 0;
	पूर्णाङ्क box_y = 0;

	box = html->drag_owner.textarea;

	प्रतिज्ञा(box->gadget != रिक्त);
	प्रतिज्ञा(box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_PASSWORD ||
	       box->gadget->type == GADGET_TEXTBOX);

	box_coords(box, &box_x, &box_y);
	textarea_mouse_action(box->gadget->दत्त.text.ta,
			      mouse,
			      x - box_x,
			      y - box_y);

	/* TODO: Set appropriate statusbar message */
	प्रत्यागम NSERROR_OK;
}


/**
 * handle mouse actions while dragging in a content
 */
स्थिर nserror
mouse_action_drag_content(html_content *html,
			  संरचना browser_गवाक्षः *bw,
			  browser_mouse_state mouse,
			  पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना box *box;
	पूर्णाङ्क box_x = 0;
	पूर्णाङ्क box_y = 0;

	box = html->drag_owner.content;
	प्रतिज्ञा(box->object != रिक्त);

	box_coords(box, &box_x, &box_y);
	content_mouse_track(box->object,
			    bw, mouse,
			    x - box_x,
			    y - box_y);
	प्रत्यागम NSERROR_OK;
}


/**
 * local structure containing all the mouse action state information
 */
संरचना mouse_action_state {
	संरचना {
		ध्रुव वर्ण *स्थितिः; /**< status text */
		browser_pointer_shape pointer; /**< pointer shape */
		गणना {
		      ACTION_NONE, /**< default of no action */
		      ACTION_NOSEND, /**< do not send status and pointer message */
		      ACTION_SUBMIT, /**< submit form */
		      ACTION_NAVIGATE, /**< navigate to link url */
		      ACTION_JS, /**< execute link as script */
		      ACTION_पृष्ठभागः, /**< navigate back in history */
		      ACTION_FORWARD, /**< navigate forward in history */
		} action;
	} परिणाम;

	/** dom node */
	संरचना dom_ग्रन्थिः *ग्रन्थिः;

	/** html object */
	संरचना {
		संरचना box *box;
		पूर्णाङ्क pos_x;
		पूर्णाङ्क pos_y;
	} html_object;

	/** non html object */
	hlcache_handle *object;

	/** iframe */
	संरचना browser_गवाक्षः *iframe;

	/** link either from href or imagemap */
	संरचना {
		संरचना box *box;
		nsurl *url;
		ध्रुव वर्ण *target;
		bool is_imagemap;
	} link;

	/** gadget */
	संरचना {
		संरचना form_control *control;
		संरचना box *box;
		पूर्णाङ्क box_x;
		पूर्णाङ्क box_y;
		ध्रुव वर्ण *target;
	} gadget;

	/** title */
	ध्रुव वर्ण *title;

	/** candidate box for drag operation */
	संरचना box *drag_candidate;

	/** scrollbar */
	संरचना {
		संरचना scrollbar *bar;
		पूर्णाङ्क mouse_x;
		पूर्णाङ्क mouse_y;
	} scroll;

	/** text in box */
	संरचना {
		संरचना box *box;
		पूर्णाङ्क box_x;
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
get_mouse_action_ग्रन्थिः(html_content *html,
		      पूर्णाङ्क x, पूर्णाङ्क y,
		      संरचना mouse_action_state *man)
{
	संरचना box *box;
	पूर्णाङ्क box_x = 0;
	पूर्णाङ्क box_y = 0;

	/* initialise the mouse action state data */
	स्मृतिं_पूरय(man, 0, परिमाण(संरचना mouse_action_state));
	man->ग्रन्थिः = html->layout->ग्रन्थिः; /* Default dom node to the <HTML> */
	man->परिणाम.pointer = BROWSER_POINTER_DEFAULT;

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars
	 */
	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[वामः_2];
	box_y = box->margin[शिखरम्_2];

	कुरु {
		/* skip hidden boxes */
		यदि ((box->style != रिक्त) &&
		    (css_computed_visibility(box->style) ==
		     CSS_VISIBILITY_HIDDEN)) {
			गच्छ अग्र_box;
		}

		यदि (box->ग्रन्थिः != रिक्त) {
			man->ग्रन्थिः = box->ग्रन्थिः;
		}

		यदि (box->object) {
			यदि (content_get_type(box->object) == CONTENT_HTML) {
				man->html_object.box = box;
				man->html_object.pos_x = box_x;
				man->html_object.pos_y = box_y;
			} अन्यथा {
				man->object = box->object;
			}
		}

		यदि (box->iframe) {
			man->iframe = box->iframe;
		}

		यदि (box->href) {
			man->link.url = box->href;
			man->link.target = box->target;
			man->link.box = box;
			man->link.is_imagemap = false;
		}

		यदि (box->usemap) {
			man->link.url = imagemap_get(html,
						     box->usemap,
						     box_x,
						     box_y,
						     x, y,
						     &man->link.target);
			man->link.box = box;
			man->link.is_imagemap = true;
		}

		यदि (box->gadget) {
			man->gadget.control = box->gadget;
			man->gadget.box = box;
			man->gadget.box_x = box_x;
			man->gadget.box_y = box_y;
			यदि (box->gadget->form) {
				man->gadget.target = box->gadget->form->target;
			}
		}

		यदि (box->title) {
			man->title = box->title;
		}

		man->परिणाम.pointer = get_pointer_shape(box, false);

		यदि ((box->scroll_x != रिक्त) ||
		    (box->scroll_y != रिक्त)) {
			पूर्णाङ्क padding_वामः;
			पूर्णाङ्क padding_दक्षिणः;
			पूर्णाङ्क padding_शिखरम्;
			पूर्णाङ्क padding_bottom;

			यदि (man->drag_candidate == रिक्त) {
				man->drag_candidate = box;
			}

			padding_वामः = box_x +
					scrollbar_get_विस्थापनम्(box->scroll_x);
			padding_दक्षिणः = padding_वामः + box->padding[वामः_2] +
					box->विस्तारः + box->padding[दक्षिणः_2];
			padding_शिखरम् = box_y +
					scrollbar_get_विस्थापनम्(box->scroll_y);
			padding_bottom = padding_शिखरम् + box->padding[शिखरम्_2] +
					box->height + box->padding[BOTTOM];

			यदि ((x > padding_वामः) &&
			    (x < padding_दक्षिणः) &&
			    (y > padding_शिखरम्) &&
			    (y < padding_bottom)) {
				/* mouse inside padding box */

				यदि ((box->scroll_y != रिक्त) &&
				    (x > (padding_दक्षिणः - SCROLLBAR_विस्तारः))) {
					/* mouse above vertical box scroll */

					man->scroll.bar = box->scroll_y;
					man->scroll.mouse_x = x - (padding_दक्षिणः - SCROLLBAR_विस्तारः);
					man->scroll.mouse_y = y - padding_शिखरम्;
					भङ्ग;

				} अन्यथा यदि ((box->scroll_x != रिक्त) &&
					   (y > (padding_bottom -
							SCROLLBAR_विस्तारः))) {
					/* mouse above horizontal box scroll */

					man->scroll.bar = box->scroll_x;
					man->scroll.mouse_x = x - padding_वामः;
					man->scroll.mouse_y = y - (padding_bottom - SCROLLBAR_विस्तारः);
					भङ्ग;
				}
			}
		}

		यदि (box->text && !box->object) {
			man->text.box = box;
			man->text.box_x = box_x;
		}

	अग्र_box:
		/* iterate to next box */
		box = box_at_point(&html->unit_len_ctx, box, x, y, &box_x, &box_y);
	} यावत् (box != रिक्त);

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	प्रतिज्ञा(man->ग्रन्थिः != रिक्त);

	प्रत्यागम NSERROR_OK;
}


/**
 * process mouse activity on a form gadget
 */
स्थिर nserror
gadget_mouse_action(html_content *html,
		    browser_mouse_state mouse,
		    पूर्णाङ्क x, पूर्णाङ्क y,
		    संरचना mouse_action_state *mas)
{
	संरचना content *c = (संरचना content *)html;
	textarea_mouse_स्थितिः ta_स्थितिः;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	nserror res;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	विकल्प (mas->gadget.control->type) {
	पक्ष GADGET_SELECT:
		mas->परिणाम.स्थितिः = messages_get("FormSelect");
		mas->परिणाम.pointer = BROWSER_POINTER_MENU;
		यदि (mouse & BROWSER_MOUSE_CLICK_1 &&
		    nsoption_bool(core_select_menu)) {
			html->visible_select_menu = mas->gadget.control;
			res = form_open_select_menu(c,
						    mas->gadget.control,
						    form_select_menu_callback,
						    c);
			यदि (res != NSERROR_OK) {
				NSLOG(netsurf, ERROR, "%s",
				      messages_get_errorcode(res));
				html->visible_select_menu = रिक्त;
			}
			mas->परिणाम.pointer = BROWSER_POINTER_DEFAULT;
		} अन्यथा यदि (mouse & BROWSER_MOUSE_CLICK_1) {
			सन्देशः_दत्त.select_menu.gadget = mas->gadget.control;
			content_प्रसारणम्(c,
					  CONTENT_सन्देशः_SELECTMENU,
					  &सन्देशः_दत्त);
		}
		भङ्ग;

	पक्ष GADGET_CHECKBOX:
		mas->परिणाम.स्थितिः = messages_get("FormCheckbox");
		यदि (mouse & BROWSER_MOUSE_CLICK_1) {
			mas->gadget.control->selected = !mas->gadget.control->selected;
			dom_html_निवेशः_element_समुच्चयः_checked(
				(dom_html_निवेशः_element *)(mas->gadget.control->ग्रन्थिः),
				mas->gadget.control->selected);
			html__redraw_a_box(html, mas->gadget.box);
		}
		भङ्ग;

	पक्ष GADGET_RADIO:
		mas->परिणाम.स्थितिः = messages_get("FormRadio");
		यदि (mouse & BROWSER_MOUSE_CLICK_1) {
			form_radio_समुच्चयः(mas->gadget.control);
		}
		भङ्ग;

	पक्ष GADGET_IMAGE:
		/* This falls through to SUBMIT */
		यदि (mouse & BROWSER_MOUSE_CLICK_1) {
			संरचना image_निवेशः_coords *coords, *oldcoords;
			/** \todo Find a way to not ignore errors */
			coords = शून्यस्मृतिं_देहि(1, परिमाण(*coords));
			यदि (coords == रिक्त) {
				प्रत्यागम NSERROR_OK;
			}
			coords->x = x - mas->gadget.box_x;
			coords->y = y - mas->gadget.box_y;
			यदि (dom_ग्रन्थिः_समुच्चयः_user_दत्त(
				mas->gadget.control->ग्रन्थिः,
				corestring_dom_ns_कुञ्चिका_image_coords_ग्रन्थिः_दत्त,
				coords,
				html_image_coords_dom_user_दत्त_handler,
				&oldcoords) != DOM_NO_ERR) {
				प्रत्यागम NSERROR_OK;
			}
			मुक्त(oldcoords);
		}
		fallthrough;

	पक्ष GADGET_SUBMIT:
		यदि (mas->gadget.control->form) {
			स्थिर वर्ण स्थितिः_बफर[200];

			snprintf(स्थितिः_बफर,
				 परिमाण स्थितिः_बफर,
				 messages_get("FormSubmit"),
				 mas->gadget.control->form->action);
			mas->परिणाम.स्थितिः = स्थितिः_बफर;
			mas->परिणाम.pointer = get_pointer_shape(mas->gadget.box,
								false);
			यदि (mouse & (BROWSER_MOUSE_CLICK_1 |
				     BROWSER_MOUSE_CLICK_2)) {
				mas->परिणाम.action = ACTION_SUBMIT;
			}
		} अन्यथा {
			mas->परिणाम.स्थितिः = messages_get("FormBadSubmit");
		}
		भङ्ग;

	पक्ष GADGET_TEXTBOX:
	पक्ष GADGET_PASSWORD:
	पक्ष GADGET_TEXTAREA:
		यदि (mas->gadget.control->type == GADGET_TEXTAREA) {
			mas->परिणाम.स्थितिः = messages_get("FormTextarea");
		} अन्यथा {
			mas->परिणाम.स्थितिः = messages_get("FormTextbox");
		}

		यदि (click &&
		    (html->selection_type != HTML_SELECTION_TEXTAREA ||
		     html->selection_owner.textarea != mas->gadget.box)) {
			सङ्घ html_selection_owner sel_owner;
			sel_owner.none = true;
			html_समुच्चयः_selection(html,
					   HTML_SELECTION_NONE,
					   sel_owner,
					   true);
		}

		ta_स्थितिः = textarea_mouse_action(mas->gadget.control->दत्त.text.ta,
						  mouse,
						  x - mas->gadget.box_x,
						  y - mas->gadget.box_y);

		यदि (ta_स्थितिः & TEXTAREA_MOUSE_EDITOR) {
			mas->परिणाम.pointer = get_pointer_shape(mas->gadget.box, false);
		} अन्यथा {
			mas->परिणाम.pointer = BROWSER_POINTER_DEFAULT;
			mas->परिणाम.स्थितिः = scrollbar_mouse_स्थितिः_गन्तव्यम्_सन्देशः(ta_स्थितिः >> 3);
		}
		भङ्ग;

	पक्ष GADGET_HIDDEN:
		/* not possible: no box generated */
		भङ्ग;

	पक्ष GADGET_RESET:
		mas->परिणाम.स्थितिः = messages_get("FormReset");
		भङ्ग;

	पक्ष GADGET_FILE:
		mas->परिणाम.स्थितिः = messages_get("FormFile");
		यदि (mouse & BROWSER_MOUSE_CLICK_1) {
			सन्देशः_दत्त.gadget_click.gadget = mas->gadget.control;
			content_प्रसारणम्(c,
					  CONTENT_सन्देशः_GADGETCLICK,
					  &सन्देशः_दत्त);
		}
		भङ्ग;

	पक्ष GADGET_BUTTON:
		/* This gadget cannot be activated */
		mas->परिणाम.स्थितिः = messages_get("FormButton");
		भङ्ग;
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * process mouse activity on an iframe
 */
स्थिर nserror
iframe_mouse_action(संरचना browser_गवाक्षः *bw,
		    browser_mouse_state mouse,
		    पूर्णाङ्क x, पूर्णाङ्क y,
		    संरचना mouse_action_state *mas)
{
	पूर्णाङ्क pos_x, pos_y;
	भिन्न scale;

	scale = browser_गवाक्षः_get_scale(bw);

	browser_गवाक्षः_get_position(mas->iframe, false, &pos_x, &pos_y);

	यदि (mouse & BROWSER_MOUSE_CLICK_1 ||
	    mouse & BROWSER_MOUSE_CLICK_2) {
		browser_गवाक्षः_mouse_click(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	} अन्यथा {
		browser_गवाक्षः_mouse_track(mas->iframe,
					   mouse,
					   (x * scale) - pos_x,
					   (y * scale) - pos_y);
	}
	mas->परिणाम.action = ACTION_NOSEND;

	प्रत्यागम NSERROR_OK;
}


/**
 * process mouse activity on an html object
 */
स्थिर nserror
html_object_mouse_action(html_content *html,
			 संरचना browser_गवाक्षः *bw,
			 browser_mouse_state mouse,
			 पूर्णाङ्क x, पूर्णाङ्क y,
			 संरचना mouse_action_state *mas)
{
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	यदि (click &&
	    (html->selection_type != HTML_SELECTION_CONTENT ||
	     html->selection_owner.content != mas->html_object.box)) {
		सङ्घ html_selection_owner sel_owner;
		sel_owner.none = true;
		html_समुच्चयः_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	यदि (mouse & BROWSER_MOUSE_CLICK_1 ||
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
	प्रत्यागम NSERROR_OK;
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
	यदि (scheme != रिक्त) {
		यदि (scheme == corestring_lwc_javascript) {
			is_js = true;
		}
		lwc_string_unref(scheme);
	}
	प्रत्यागम is_js;
}


/**
 * process mouse activity on a link
 */
स्थिर nserror
link_mouse_action(html_content *html,
		  संरचना browser_गवाक्षः *bw,
		  browser_mouse_state mouse,
		  पूर्णाङ्क x, पूर्णाङ्क y,
		  संरचना mouse_action_state *mas)
{
	nserror res;
	वर्ण *url_s = रिक्त;
	size_t url_l = 0;
	स्थिर वर्ण स्थितिः_बफर[200];
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;

	यदि (nsoption_bool(display_decoded_idn) == true) {
		res = nsurl_get_utf8(mas->link.url, &url_s, &url_l);
		यदि (res != NSERROR_OK) {
			/* Unable to obtain a decoded IDN. This is not
			 *  a fatal error.  Ensure the string pointer
			 *  is NULL so we use the encoded version.
			 */
			url_s = रिक्त;
		}
	}

	यदि (mas->title) {
		snprintf(स्थितिः_बफर,
			 परिमाण स्थितिः_बफर,
			 "%s: %s",
			 url_s ? url_s : nsurl_access(mas->link.url),
			 mas->title);
	} अन्यथा {
		snprintf(स्थितिः_बफर,
			 परिमाण स्थितिः_बफर,
			 "%s",
			 url_s ? url_s : nsurl_access(mas->link.url));
	}

	यदि (url_s != रिक्त) {
		मुक्त(url_s);
	}

	mas->परिणाम.स्थितिः = स्थितिः_बफर;

	mas->परिणाम.pointer = get_pointer_shape(mas->link.box,
						mas->link.is_imagemap);

	यदि (mouse & BROWSER_MOUSE_CLICK_1 &&
	    mouse & BROWSER_MOUSE_MOD_1) {
		/* force download of link */
		browser_गवाक्षः_navigate(bw,
					mas->link.url,
					content_get_url((संरचना content *)html),
					BW_NAVIGATE_DOWNLOAD,
					रिक्त,
					रिक्त,
					रिक्त);

	} अन्यथा यदि (mouse & BROWSER_MOUSE_CLICK_2 &&
		   mouse & BROWSER_MOUSE_MOD_1) {
		सन्देशः_दत्त.savelink.url = mas->link.url;
		सन्देशः_दत्त.savelink.title = mas->title;
		content_प्रसारणम्((संरचना content *)html,
				  CONTENT_सन्देशः_SAVELINK,
				  &सन्देशः_दत्त);

	} अन्यथा यदि (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)) {
		यदि (is_javascript_navigate_url(mas->link.url)) {
			mas->परिणाम.action = ACTION_JS;
		} अन्यथा {
			mas->परिणाम.action = ACTION_NAVIGATE;
		}
	}

	प्रत्यागम NSERROR_OK;
}


स्थिर nserror
default_mouse_action_focus(html_content *html, browser_mouse_state mouse)
{
	यदि (mouse && mouse < BROWSER_MOUSE_MOD_1) {
		/* ensure key presses still act on the browser window */
		सङ्घ html_focus_owner fo;
		fo.self = true;
		html_समुच्चयः_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, रिक्त);
	}

	प्रत्यागम NSERROR_OK;
}


/**
 * process mouse activity if it is not anything else
 */
स्थिर nserror
default_mouse_action(html_content *html,
		  संरचना browser_गवाक्षः *bw,
		  browser_mouse_state mouse,
		  पूर्णाङ्क x, पूर्णाङ्क y,
		  संरचना mouse_action_state *mas)
{
	संरचना content *c = (संरचना content *)html;

	/* frame resizing */
	यदि (browser_गवाक्षः_frame_resize_start(bw, mouse, x, y, &mas->परिणाम.pointer)) {
		यदि (mouse & (BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2)) {
			mas->परिणाम.स्थितिः = messages_get("FrameDrag");
		}
		प्रत्यागम default_mouse_action_focus(html, mouse);
	}

	/* clicking in the main page removes the selection from any text areas.
	 */
	सङ्घ html_selection_owner sel_owner;
	bool click;
	click = mouse & (BROWSER_MOUSE_PRESS_1 | BROWSER_MOUSE_PRESS_2 |
			 BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2 |
			 BROWSER_MOUSE_DRAG_1 | BROWSER_MOUSE_DRAG_2);

	यदि (click && html->focus_type != HTML_FOCUS_SELF) {
		सङ्घ html_focus_owner fo;
		fo.self = true;
		html_समुच्चयः_focus(html, HTML_FOCUS_SELF, fo, true, 0, 0, 0, रिक्त);
	}
	यदि (click && html->selection_type != HTML_SELECTION_SELF) {
		sel_owner.none = true;
		html_समुच्चयः_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	यदि (mas->text.box) {
		पूर्णाङ्क pixel_विस्थापनम्;
		size_t idx;
		plot_font_style_t fstyle;

		font_plot_style_from_css(&html->unit_len_ctx,
					 mas->text.box->style,
					 &fstyle);

		guit->layout->position(&fstyle,
				       mas->text.box->text,
				       mas->text.box->दीर्घता,
				       x - mas->text.box_x,
				       &idx,
				       &pixel_विस्थापनम्);

		यदि (selection_click(html->sel,
				    html->bw,
				    mouse,
				    mas->text.box->byte_विस्थापनम् + idx)) {
			/* key presses must be directed at the
			 * main browser window, paste text
			 * operations ignored */
			html_drag_type drag_type;
			सङ्घ html_drag_owner drag_owner;

			यदि (selection_dragging(html->sel)) {
				drag_type = HTML_DRAG_SELECTION;
				drag_owner.no_owner = true;
				html_समुच्चयः_drag_type(html,
						   drag_type,
						   drag_owner,
						   रिक्त);
				mas->परिणाम.स्थितिः = messages_get("Selecting");
			}

			यदि (selection_सक्रिय(html->sel)) {
				sel_owner.none = false;
				html_समुच्चयः_selection(html, HTML_SELECTION_SELF,
						   sel_owner, true);
			} अन्यथा यदि (click && html->selection_type != HTML_SELECTION_NONE) {
				sel_owner.none = true;
				html_समुच्चयः_selection(html, HTML_SELECTION_NONE,
						   sel_owner, true);
			}

			प्रत्यागम default_mouse_action_focus(html, mouse);
		}

	} अन्यथा यदि (mouse & BROWSER_MOUSE_PRESS_1) {
		sel_owner.none = true;
		selection_clear(html->sel, true);
	}

	यदि (selection_सक्रिय(html->sel)) {
		sel_owner.none = false;
		html_समुच्चयः_selection(html, HTML_SELECTION_SELF, sel_owner, true);
	} अन्यथा यदि (click && html->selection_type != HTML_SELECTION_NONE) {
		sel_owner.none = true;
		html_समुच्चयः_selection(html, HTML_SELECTION_NONE, sel_owner, true);
	}

	यदि (mas->title) {
		mas->परिणाम.स्थितिः = mas->title;
	}

	यदि (mouse & BROWSER_MOUSE_DRAG_1) {
		यदि (mouse & BROWSER_MOUSE_MOD_2) {
			सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
			सन्देशः_दत्त.dragsave.type = CONTENT_SAVE_COMPLETE;
			सन्देशः_दत्त.dragsave.content = रिक्त;
			content_प्रसारणम्(c, CONTENT_सन्देशः_DRAGSAVE, &सन्देशः_दत्त);
		} अन्यथा {
			यदि (mas->drag_candidate == रिक्त) {
				browser_गवाक्षः_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->परिणाम.pointer = BROWSER_POINTER_MOVE;
		}
	} अन्यथा यदि (mouse & BROWSER_MOUSE_DRAG_2) {
		यदि (mouse & BROWSER_MOUSE_MOD_2) {
			सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
			सन्देशः_दत्त.dragsave.type = CONTENT_SAVE_SOURCE;
			सन्देशः_दत्त.dragsave.content = रिक्त;
			content_प्रसारणम्(c, CONTENT_सन्देशः_DRAGSAVE, &सन्देशः_दत्त);
		} अन्यथा {
			यदि (mas->drag_candidate == रिक्त) {
				browser_गवाक्षः_page_drag_start(bw, x, y);
			} अन्यथा {
				html_box_drag_start(mas->drag_candidate, x, y);
			}
			mas->परिणाम.pointer = BROWSER_POINTER_MOVE;
		}
	}


	प्रत्यागम default_mouse_action_focus(html, mouse);
}


/**
 * handle non dragging mouse actions
 */
स्थिर nserror
mouse_action_drag_none(html_content *html,
		       संरचना browser_गवाक्षः *bw,
		       browser_mouse_state mouse,
		       पूर्णाङ्क x, पूर्णाङ्क y)
{
	nserror res;
	संरचना content *c = (संरचना content *)html;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	lwc_string *मार्गः;

	/**
	 * computed state
	 *
	 * not on heap to avoid allocation or stack because it is large
	 */
	स्थिर संरचना mouse_action_state mas;

	res = get_mouse_action_ग्रन्थिः(html, x, y, &mas);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	यदि (mouse & BROWSER_MOUSE_CLICK_4) {
		mas.परिणाम.action = ACTION_पृष्ठभागः;
	} अन्यथा यदि (mouse & BROWSER_MOUSE_CLICK_5) {
		mas.परिणाम.action = ACTION_FORWARD;
	} अन्यथा यदि (mas.scroll.bar) {
		mas.परिणाम.स्थितिः = scrollbar_mouse_स्थितिः_गन्तव्यम्_सन्देशः(
				scrollbar_mouse_action(mas.scroll.bar,
						       mouse,
						       mas.scroll.mouse_x,
						       mas.scroll.mouse_y));
		mas.परिणाम.pointer = BROWSER_POINTER_DEFAULT;

	} अन्यथा यदि (mas.gadget.control) {
		res = gadget_mouse_action(html, mouse, x, y, &mas);

	} अन्यथा यदि ((mas.object != रिक्त) && (mouse & BROWSER_MOUSE_MOD_2)) {

		यदि (mouse & BROWSER_MOUSE_DRAG_2) {
			सन्देशः_दत्त.dragsave.type = CONTENT_SAVE_NATIVE;
			सन्देशः_दत्त.dragsave.content = mas.object;
			content_प्रसारणम्(c, CONTENT_सन्देशः_DRAGSAVE, &सन्देशः_दत्त);

		} अन्यथा यदि (mouse & BROWSER_MOUSE_DRAG_1) {
			सन्देशः_दत्त.dragsave.type = CONTENT_SAVE_ORIG;
			सन्देशः_दत्त.dragsave.content = mas.object;
			content_प्रसारणम्(c, CONTENT_सन्देशः_DRAGSAVE, &सन्देशः_दत्त);
		}

		/* \todo should have a drag-saving object msg */

	} अन्यथा यदि (mas.iframe != रिक्त) {
		res = iframe_mouse_action(bw, mouse, x, y, &mas);

	} अन्यथा यदि (mas.html_object.box != रिक्त) {
		res = html_object_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा यदि (mas.link.url != रिक्त) {
		res = link_mouse_action(html, bw, mouse, x, y, &mas);

	} अन्यथा {
		res = default_mouse_action(html, bw, mouse, x, y, &mas);

	}
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	/* send status and pointer message */
	यदि (mas.परिणाम.action != ACTION_NOSEND) {
		सन्देशः_दत्त.explicit_स्थितिः_text = mas.परिणाम.स्थितिः;
		content_प्रसारणम्(c, CONTENT_सन्देशः_स्थितिः, &सन्देशः_दत्त);

		सन्देशः_दत्त.pointer = mas.परिणाम.pointer;
		content_प्रसारणम्(c, CONTENT_सन्देशः_POINTER, &सन्देशः_दत्त);
	}

	/* fire dom click event */
	यदि (mouse & BROWSER_MOUSE_CLICK_1) {
		fire_generic_dom_event(corestring_dom_click, mas.ग्रन्थिः, true, true);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	विकल्प (mas.परिणाम.action) {
	पक्ष ACTION_SUBMIT:
		res = form_submit(content_get_url(c),
				  browser_गवाक्षः_find_target(bw,
							     mas.gadget.target,
							     mouse),
				  mas.gadget.control->form,
				  mas.gadget.control);
		भङ्ग;

	पक्ष ACTION_NAVIGATE:
		res = browser_गवाक्षः_navigate(
				browser_गवाक्षः_find_target(bw,
							   mas.link.target,
							   mouse),
				mas.link.url,
				content_get_url(c),
				BW_NAVIGATE_HISTORY,
				रिक्त,
				रिक्त,
				रिक्त);
		भङ्ग;

	पक्ष ACTION_JS:
		मार्गः = nsurl_get_component(mas.link.url, NSURL_मार्गः);
		यदि (मार्गः != रिक्त) {
			html_exec(c,
				  lwc_string_दत्त(मार्गः),
				  lwc_string_दीर्घता(मार्गः));
			lwc_string_unref(मार्गः);
		}
		भङ्ग;

	पक्ष ACTION_पृष्ठभागः:
		res = browser_गवाक्षः_history_पृष्ठभागः(bw, false);
		भङ्ग;

	पक्ष ACTION_FORWARD:
		res = browser_गवाक्षः_history_forward(bw, false);
		भङ्ग;

	पक्ष ACTION_NOSEND:
	पक्ष ACTION_NONE:
		res = NSERROR_OK;
		भङ्ग;
	}

	प्रत्यागम res;
}


/* exported interface documented in html/interaction.h */
nserror html_mouse_track(संरचना content *c,
			 संरचना browser_गवाक्षः *bw,
			 browser_mouse_state mouse,
			 पूर्णाङ्क x, पूर्णाङ्क y)
{
	प्रत्यागम html_mouse_action(c, bw, mouse, x, y);
}


/* exported interface documented in html/interaction.h */
nserror
html_mouse_action(संरचना content *c,
		  संरचना browser_गवाक्षः *bw,
		  browser_mouse_state mouse,
		  पूर्णाङ्क x, पूर्णाङ्क y)
{
	html_content *html = (html_content *)c;
	nserror res = NSERROR_OK;

	/* handle open select menu */
	यदि (html->visible_select_menu != रिक्त) {
		प्रत्यागम mouse_action_select_menu(html, bw, mouse, x, y);
	}

	/* handle content drag */
	विकल्प (html->drag_type) {
	पक्ष HTML_DRAG_SELECTION:
		res = mouse_action_drag_selection(html, bw, mouse, x, y);
		भङ्ग;

	पक्ष HTML_DRAG_SCROLLBAR:
		res = mouse_action_drag_scrollbar(html, bw, mouse, x, y);
		भङ्ग;

	पक्ष HTML_DRAG_TEXTAREA_SELECTION:
	पक्ष HTML_DRAG_TEXTAREA_SCROLLBAR:
		res = mouse_action_drag_textarea(html, bw, mouse, x, y);
		भङ्ग;

	पक्ष HTML_DRAG_CONTENT_SELECTION:
	पक्ष HTML_DRAG_CONTENT_SCROLL:
		res = mouse_action_drag_content(html, bw, mouse, x, y);
		भङ्ग;

	पक्ष HTML_DRAG_NONE:
		res = mouse_action_drag_none(html, bw, mouse, x, y);
		भङ्ग;

	स्वभाव:
		/* Unknown content related drag type */
		प्रतिज्ञा(0 && "Unknown content related drag type");
	}

	यदि (res != NSERROR_OK) {
		NSLOG(netsurf, ERROR, "%s", messages_get_errorcode(res));
	}

	प्रत्यागम res;
}


/**
 * Handle keypresses.
 *
 * \param  c	content of type HTML
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool html_keypress(संरचना content *c, uint32_t कुञ्चिका)
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
	यदि (html->layout != रिक्त && html->layout->ग्रन्थिः != रिक्त) {
		fire_dom_keyboard_event(corestring_dom_keydown,
				html->layout->ग्रन्थिः, true, true, कुञ्चिका);
	}

	विकल्प (html->focus_type) {
	पक्ष HTML_FOCUS_CONTENT:
		प्रत्यागम content_keypress(html->focus_owner.content->object, कुञ्चिका);

	पक्ष HTML_FOCUS_TEXTAREA:
		यदि (box_textarea_keypress(html, html->focus_owner.textarea, कुञ्चिका) == NSERROR_OK) {
			प्रत्यागम true;
		} अन्यथा {
			प्रत्यागम false;
		}

	स्वभाव:
		/* Deal with it below */
		भङ्ग;
	}

	विकल्प (कुञ्चिका) {
	पक्ष NS_कुञ्चिका_COPY_SELECTION:
		selection_copy_गन्तव्यम्_clipboard(sel);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_CLEAR_SELECTION:
		selection_clear(sel, true);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_SELECT_ALL:
		selection_select_all(sel);
		प्रत्यागम true;

	पक्ष NS_कुञ्चिका_ESCAPE:
		/* if there's no selection, leave Escape for the caller */
		प्रत्यागम selection_clear(sel, true);
	}

	प्रत्यागम false;
}


/**
 * Callback for in-page scrollbars.
 */
शून्य html_अतिप्रवाहः_scroll_callback(शून्य *client_दत्त,
		संरचना scrollbar_सन्देशः_दत्त *scrollbar_दत्त)
{
	संरचना html_scrollbar_दत्त *दत्त = client_दत्त;
	html_content *html = (html_content *)दत्त->c;
	संरचना box *box = दत्त->box;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	html_drag_type drag_type;
	सङ्घ html_drag_owner drag_owner;

	विकल्प(scrollbar_दत्त->सन्देशः) {
	पक्ष SCROLLBAR_सन्देशः_MOVED:

		यदि (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			भङ्ग;
		}

		html__redraw_a_box(html, box);
		भङ्ग;
	पक्ष SCROLLBAR_सन्देशः_SCROLL_START:
	{
		संरचना rect rect = {
			.x0 = scrollbar_दत्त->x0,
			.y0 = scrollbar_दत्त->y0,
			.x1 = scrollbar_दत्त->x1,
			.y1 = scrollbar_दत्त->y1
		};
		drag_type = HTML_DRAG_SCROLLBAR;
		drag_owner.scrollbar = scrollbar_दत्त->scrollbar;
		html_समुच्चयः_drag_type(html, drag_type, drag_owner, &rect);
	}
		भङ्ग;
	पक्ष SCROLLBAR_सन्देशः_SCROLL_परिसमाप्तम्:
		drag_type = HTML_DRAG_NONE;
		drag_owner.no_owner = true;
		html_समुच्चयः_drag_type(html, drag_type, drag_owner, रिक्त);

		सन्देशः_दत्त.pointer = BROWSER_POINTER_AUTO;
		content_प्रसारणम्(दत्त->c, CONTENT_सन्देशः_POINTER, &सन्देशः_दत्त);
		भङ्ग;
	}
}


/* Documented in html_internal.h */
शून्य html_समुच्चयः_drag_type(html_content *html, html_drag_type drag_type,
		सङ्घ html_drag_owner drag_owner, ध्रुव संरचना rect *rect)
{
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;

	प्रतिज्ञा(html != रिक्त);

	html->drag_type = drag_type;
	html->drag_owner = drag_owner;

	विकल्प (drag_type) {
	पक्ष HTML_DRAG_NONE:
		प्रतिज्ञा(drag_owner.no_owner == true);
		सन्देशः_दत्त.drag.type = CONTENT_DRAG_NONE;
		भङ्ग;

	पक्ष HTML_DRAG_SCROLLBAR:
	पक्ष HTML_DRAG_TEXTAREA_SCROLLBAR:
	पक्ष HTML_DRAG_CONTENT_SCROLL:
		सन्देशः_दत्त.drag.type = CONTENT_DRAG_SCROLL;
		भङ्ग;

	पक्ष HTML_DRAG_SELECTION:
		प्रतिज्ञा(drag_owner.no_owner == true);
		fallthrough;
	पक्ष HTML_DRAG_TEXTAREA_SELECTION:
	पक्ष HTML_DRAG_CONTENT_SELECTION:
		सन्देशः_दत्त.drag.type = CONTENT_DRAG_SELECTION;
		भङ्ग;
	}
	सन्देशः_दत्त.drag.rect = rect;

	/* Inform of the content's drag status change */
	content_प्रसारणम्((संरचना content *)html, CONTENT_सन्देशः_DRAG, &सन्देशः_दत्त);
}

/* Documented in html_internal.h */
शून्य html_समुच्चयः_focus(html_content *html, html_focus_type focus_type,
		सङ्घ html_focus_owner focus_owner, bool hide_caret,
		पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क height, ध्रुव संरचना rect *clip)
{
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	पूर्णाङ्क x_off = 0;
	पूर्णाङ्क y_off = 0;
	संरचना rect cr;
	bool textarea_नष्टम्_focus = html->focus_type == HTML_FOCUS_TEXTAREA &&
			focus_type != HTML_FOCUS_TEXTAREA;

	प्रतिज्ञा(html != रिक्त);

	विकल्प (focus_type) {
	पक्ष HTML_FOCUS_SELF:
		प्रतिज्ञा(focus_owner.self == true);
		यदि (html->focus_type == HTML_FOCUS_SELF)
			/* Don't need to tell anyone anything */
			प्रत्यागम;
		भङ्ग;

	पक्ष HTML_FOCUS_CONTENT:
		box_coords(focus_owner.content, &x_off, &y_off);
		भङ्ग;

	पक्ष HTML_FOCUS_TEXTAREA:
		box_coords(focus_owner.textarea, &x_off, &y_off);
		भङ्ग;
	}

	html->focus_type = focus_type;
	html->focus_owner = focus_owner;

	यदि (textarea_नष्टम्_focus) {
		सन्देशः_दत्त.caret.type = CONTENT_CARET_REMOVE;
	} अन्यथा यदि (focus_type != HTML_FOCUS_SELF && hide_caret) {
		सन्देशः_दत्त.caret.type = CONTENT_CARET_HIDE;
	} अन्यथा {
		यदि (clip != रिक्त) {
			cr = *clip;
			cr.x0 += x_off;
			cr.y0 += y_off;
			cr.x1 += x_off;
			cr.y1 += y_off;
		}

		सन्देशः_दत्त.caret.type = CONTENT_CARET_समुच्चयः_POS;
		सन्देशः_दत्त.caret.pos.x = x + x_off;
		सन्देशः_दत्त.caret.pos.y = y + y_off;
		सन्देशः_दत्त.caret.pos.height = height;
		सन्देशः_दत्त.caret.pos.clip = (clip == रिक्त) ? रिक्त : &cr;
	}

	/* Inform of the content's drag status change */
	content_प्रसारणम्((संरचना content *)html, CONTENT_सन्देशः_CARET, &सन्देशः_दत्त);
}

/* Documented in html_internal.h */
शून्य html_समुच्चयः_selection(html_content *html, html_selection_type selection_type,
		सङ्घ html_selection_owner selection_owner, bool read_only)
{
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	संरचना box *box;
	bool परिवर्तितम् = false;
	bool same_type = html->selection_type == selection_type;

	प्रतिज्ञा(html != रिक्त);

	यदि ((selection_type == HTML_SELECTION_NONE &&
			html->selection_type != HTML_SELECTION_NONE) ||
			(selection_type != HTML_SELECTION_NONE &&
			html->selection_type == HTML_SELECTION_NONE))
		/* Existance of selection has changed, and we'll need to
		 * inform our owner */
		परिवर्तितम् = true;

	/* Clear any existing selection */
	यदि (html->selection_type != HTML_SELECTION_NONE) {
		विकल्प (html->selection_type) {
		पक्ष HTML_SELECTION_SELF:
			यदि (same_type)
				भङ्ग;
			selection_clear(html->sel, true);
			भङ्ग;
		पक्ष HTML_SELECTION_TEXTAREA:
			यदि (same_type && html->selection_owner.textarea ==
					selection_owner.textarea)
				भङ्ग;
			box = html->selection_owner.textarea;
			textarea_clear_selection(box->gadget->दत्त.text.ta);
			भङ्ग;
		पक्ष HTML_SELECTION_CONTENT:
			यदि (same_type && html->selection_owner.content ==
					selection_owner.content)
				भङ्ग;
			box = html->selection_owner.content;
			content_clear_selection(box->object);
			भङ्ग;
		स्वभाव:
			भङ्ग;
		}
	}

	html->selection_type = selection_type;
	html->selection_owner = selection_owner;

	यदि (!परिवर्तितम्)
		/* Don't need to report lack of change to owner */
		प्रत्यागम;

	/* Prepare msg */
	विकल्प (selection_type) {
	पक्ष HTML_SELECTION_NONE:
		प्रतिज्ञा(selection_owner.none == true);
		सन्देशः_दत्त.selection.selection = false;
		भङ्ग;
	पक्ष HTML_SELECTION_SELF:
		प्रतिज्ञा(selection_owner.none == false);
		fallthrough;
	पक्ष HTML_SELECTION_TEXTAREA:
	पक्ष HTML_SELECTION_CONTENT:
		सन्देशः_दत्त.selection.selection = true;
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}
	सन्देशः_दत्त.selection.read_only = read_only;

	/* Inform of the content's selection status change */
	content_प्रसारणम्((संरचना content *)html, CONTENT_सन्देशः_SELECTION,
			&सन्देशः_दत्त);
}
