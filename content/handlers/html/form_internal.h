/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
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
 * Interface to form handling functions internal to HTML content handler.
 */

#ifndef NETSURF_HTML_FORM_INTERNAL_H
#define NETSURF_HTML_FORM_INTERNAL_H

#அந்தர்பாவ <stdbool.h>

#அந்தர்பாவ "netsurf/form.h"

கட்டமைப்பு box;
கட்டமைப்பு form_control;
கட்டமைப்பு form_option;
கட்டமைப்பு form_select_menu;
கட்டமைப்பு form;
கட்டமைப்பு html_content;
கட்டமைப்பு dom_string;
கட்டமைப்பு content;
கட்டமைப்பு nsurl;
கட்டமைப்பு fetch_multipart_data;
கட்டமைப்பு redraw_context;
கட்டமைப்பு browser_window;

எண்ணிக்கை browser_mouse_state;

/** Type of a struct form_control. */
வகைவரை எண்ணிக்கை {
	GADGET_HIDDEN,
	GADGET_TEXTBOX,
	GADGET_RADIO,
	GADGET_CHECKBOX,
	GADGET_SELECT,
	GADGET_TEXTAREA,
	GADGET_IMAGE,
	GADGET_PASSWORD,
	GADGET_SUBMIT,
	GADGET_RESET,
	GADGET_FILE,
	GADGET_BUTTON
} form_control_type;

/** Data for textarea */
கட்டமைப்பு form_textarea_data {
	கட்டமைப்பு form_control *gadget;
};

கட்டமைப்பு image_உள்ளீடு_coords {
	முழு x;
	முழு y;
};

/** Form control. */
கட்டமைப்பு form_control {
	வெறுமை *முனை;			/**< Corresponding DOM node */
	கட்டமைப்பு dom_string *முனை_மதிப்பு;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	கட்டமைப்பு html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	கட்டமைப்பு form *form;		/**< Containing form */

	எழுத்து *name;			/**< Control name */
	எழுத்து *மதிப்பு;			/**< Current value of control */
	எழுத்து *initial_மதிப்பு;		/**< Initial value of control */
	எழுத்து *last_synced_மதிப்பு;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	கட்டமைப்பு box *box;		/**< Box for control */

	unsigned முழு length;		/**< Number of characters in control */
	unsigned முழு maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	ஒன்றிப்பு {
		கட்டமைப்பு {
			முழு mx, my;
		} image;
		கட்டமைப்பு {
			முழு இலக்கம்_பொருட்கள்;
			கட்டமைப்பு form_option *பொருட்கள், *last_item;
			bool multiple;
			முழு இலக்கம்_selected;
			/** Currently selected item, if num_selected == 1. */
			கட்டமைப்பு form_option *current;
			கட்டமைப்பு form_select_menu *menu;
		} select;
		கட்டமைப்பு {
			கட்டமைப்பு textarea *ta;
			கட்டமைப்பு dom_string *initial;
			கட்டமைப்பு form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	கட்டமைப்பு form_control *prev;      /**< Previous control in this form */
	கட்டமைப்பு form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
வகைவரை எண்ணிக்கை {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
கட்டமைப்பு form {
	வெறுமை *முனை;			/**< Corresponding DOM node */

	எழுத்து *action;			/**< Absolute URL to submit to. */
	எழுத்து *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	எழுத்து *accept_charsets;		/**< Charset to submit form in */
	எழுத்து *document_charset;		/**< Charset of document containing form */
	கட்டமைப்பு form_control *controls;	/**< Linked list of controls. */
	கட்டமைப்பு form_control *last_control;	/**< Last control in list. */

	கட்டமைப்பு form *prev;		/**< Previous form in doc. */
};

/**
 * Called by the select menu when it wants an area to be redrawn. The
 * coordinates are menu origin relative.
 *
 * \param client_data	data which was passed to form_open_select_menu
 * \param x		X coordinate of redraw rectangle
 * \param y		Y coordinate of redraw rectangle
 * \param width		width of redraw rectangle
 * \param height	height of redraw rectangle
 */
வகைவரை வெறுமை(*select_menu_redraw_callback)(வெறுமை *client_data,
		முழு x, முழு y, முழு width, முழு height);

/**
 * Create a struct form.
 *
 * \param node         DOM node associated with form
 * \param action       URL to submit form to, or NULL for default
 * \param target       Target frame of form, or NULL for default
 * \param method       method and enctype
 * \param charset      acceptable encodings for form submission, or NULL
 * \param doc_charset  encoding of containing document, or NULL
 * \return A new form or NULL on memory exhaustion
 */
கட்டமைப்பு form *form_new(வெறுமை *முனை, மாறிலி எழுத்து *action, மாறிலி எழுத்து *target,
		form_method method, மாறிலி எழுத்து *charset,
		மாறிலி எழுத்து *doc_charset);

/**
 * Free a form and any controls it owns.
 *
 * \note There may exist controls attached to box tree nodes which are not
 * associated with any form. These will leak at present. Ideally, they will
 * be cleaned up when the box tree is destroyed. As that currently happens
 * via talloc, this won't happen. These controls are distinguishable, as their
 * form field will be NULL.
 *
 * \param form The form to free
 */
வெறுமை form_free(கட்டமைப்பு form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
கட்டமைப்பு form_control *form_new_control(வெறுமை *முனை, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
வெறுமை form_add_control(கட்டமைப்பு form *form, கட்டமைப்பு form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
வெறுமை form_free_control(கட்டமைப்பு form_control *control);


/**
 * Add an option to a form select control.
 *
 * \param  control   form control of type GADGET_SELECT
 * \param  value     value of option, used directly (not copied)
 * \param  text      text for option, used directly (not copied)
 * \param  selected  this option is selected
 * \param  node      the DOM node this option is associated with
 * \return  true on success, false on memory exhaustion
 */
bool form_add_option(கட்டமைப்பு form_control *control, எழுத்து *மதிப்பு, எழுத்து *text,
		     bool selected, வெறுமை *முனை);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(வெறுமை *client_data,
		கட்டமைப்பு form_control *control,
		select_menu_redraw_callback redraw_callback,
		கட்டமைப்பு content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
வெறுமை form_free_select_menu(கட்டமைப்பு form_control *control);


/**
 * Redraw an opened select menu.
 *
 * \param control  the select menu being redrawn
 * \param x        the X coordinate to draw the menu at
 * \param y        the Y coordinate to draw the menu at
 * \param scale    current redraw scale
 * \param clip     clipping rectangle
 * \param ctx      current redraw context
 * \return         true on success, false otherwise
 */
bool form_redraw_select_menu(கட்டமைப்பு form_control *control, முழு x, முழு y,
		தசம scale, மாறிலி கட்டமைப்பு rect *clip,
		மாறிலி கட்டமைப்பு redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(கட்டமைப்பு form_control *control, தசம scale,
		மாறிலி கட்டமைப்பு rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
மாறிலி எழுத்து *form_select_mouse_action(கட்டமைப்பு form_control *control,
		எண்ணிக்கை browser_mouse_state mouse, முழு x, முழு y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
வெறுமை form_select_mouse_drag_end(கட்டமைப்பு form_control *control,
		எண்ணிக்கை browser_mouse_state mouse, முழு x, முழு y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
வெறுமை form_select_get_dimensions(கட்டமைப்பு form_control *control,
		முழு *width, முழு *height);


/**
 * Callback for the core select menu.
 */
வெறுமை form_select_menu_callback(வெறுமை *client_data,
		முழு x, முழு y, முழு width, முழு height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
வெறுமை form_radio_set(கட்டமைப்பு form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(கட்டமைப்பு nsurl *page_url, கட்டமைப்பு browser_window *target,
		கட்டமைப்பு form *form, கட்டமைப்பு form_control *submit_button);


/**
 * Update gadget value.
 */
வெறுமை form_gadget_update_மதிப்பு(கட்டமைப்பு form_control *control, எழுத்து *மதிப்பு);


/**
 * Synchronise this gadget with its associated DOM node.
 *
 * If the DOM has changed and the gadget has not, the DOM's new value is
 * imported into the gadget.  If the gadget's value has changed and the DOM's
 * has not, the gadget's value is pushed into the DOM.
 * If both have changed, the gadget's value wins.
 *
 * \param control The form gadget to synchronise
 *
 * \note Currently this will only synchronise input gadgets (text/password)
 */
வெறுமை form_gadget_sync_with_dom(கட்டமைப்பு form_control *control);

#endif
