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

#شامل <stdbool.h>

#شامل "netsurf/form.h"

ساختار box;
ساختار form_control;
ساختار form_option;
ساختار form_select_menu;
ساختار form;
ساختار html_content;
ساختار dom_string;
ساختار content;
ساختار nsurl;
ساختار fetch_multipart_data;
ساختار redraw_context;
ساختار browser_window;

شمارش browser_mouse_state;

/** Type of a struct form_control. */
تعریفنوع شمارش {
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
ساختار form_textarea_data {
	ساختار form_control *gadget;
};

ساختار image_درون_داد_coords {
	صحیح x;
	صحیح y;
};

/** Form control. */
ساختار form_control {
	تهی *گره;			/**< Corresponding DOM node */
	ساختار dom_string *گره_مقدار;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ساختار html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ساختار form *form;		/**< Containing form */

	نویسه *name;			/**< Control name */
	نویسه *مقدار;			/**< Current value of control */
	نویسه *initial_مقدار;		/**< Initial value of control */
	نویسه *last_synced_مقدار;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ساختار box *box;		/**< Box for control */

	unsigned صحیح length;		/**< Number of characters in control */
	unsigned صحیح maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	اجتماع {
		ساختار {
			صحیح mx, my;
		} image;
		ساختار {
			صحیح رقم_اقلام;
			ساختار form_option *اقلام, *last_item;
			bool multiple;
			صحیح رقم_selected;
			/** Currently selected item, if num_selected == 1. */
			ساختار form_option *current;
			ساختار form_select_menu *menu;
		} select;
		ساختار {
			ساختار textarea *ta;
			ساختار dom_string *initial;
			ساختار form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	ساختار form_control *prev;      /**< Previous control in this form */
	ساختار form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
تعریفنوع شمارش {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
ساختار form {
	تهی *گره;			/**< Corresponding DOM node */

	نویسه *action;			/**< Absolute URL to submit to. */
	نویسه *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	نویسه *accept_charsets;		/**< Charset to submit form in */
	نویسه *document_charset;		/**< Charset of document containing form */
	ساختار form_control *controls;	/**< Linked list of controls. */
	ساختار form_control *last_control;	/**< Last control in list. */

	ساختار form *prev;		/**< Previous form in doc. */
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
تعریفنوع تهی(*select_menu_redraw_callback)(تهی *client_data,
		صحیح x, صحیح y, صحیح width, صحیح height);

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
ساختار form *form_new(تهی *گره, ثابت نویسه *action, ثابت نویسه *target,
		form_method method, ثابت نویسه *charset,
		ثابت نویسه *doc_charset);

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
تهی form_free(ساختار form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ساختار form_control *form_new_control(تهی *گره, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
تهی form_add_control(ساختار form *form, ساختار form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
تهی form_free_control(ساختار form_control *control);


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
bool form_add_option(ساختار form_control *control, نویسه *مقدار, نویسه *text,
		     bool selected, تهی *گره);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(تهی *client_data,
		ساختار form_control *control,
		select_menu_redraw_callback redraw_callback,
		ساختار content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
تهی form_free_select_menu(ساختار form_control *control);


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
bool form_redraw_select_menu(ساختار form_control *control, صحیح x, صحیح y,
		اعشاری scale, ثابت ساختار rect *clip,
		ثابت ساختار redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ساختار form_control *control, اعشاری scale,
		ثابت ساختار rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ثابت نویسه *form_select_mouse_action(ساختار form_control *control,
		شمارش browser_mouse_state mouse, صحیح x, صحیح y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
تهی form_select_mouse_drag_end(ساختار form_control *control,
		شمارش browser_mouse_state mouse, صحیح x, صحیح y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
تهی form_select_get_dimensions(ساختار form_control *control,
		صحیح *width, صحیح *height);


/**
 * Callback for the core select menu.
 */
تهی form_select_menu_callback(تهی *client_data,
		صحیح x, صحیح y, صحیح width, صحیح height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
تهی form_radio_set(ساختار form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ساختار nsurl *page_url, ساختار browser_window *target,
		ساختار form *form, ساختار form_control *submit_button);


/**
 * Update gadget value.
 */
تهی form_gadget_update_مقدار(ساختار form_control *control, نویسه *مقدار);


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
تهی form_gadget_sync_with_dom(ساختار form_control *control);

#endif
