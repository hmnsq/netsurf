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

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ "netsurf/form.h"

ಸಂರಚನಾ box;
ಸಂರಚನಾ form_control;
ಸಂರಚನಾ form_option;
ಸಂರಚನಾ form_select_menu;
ಸಂರಚನಾ form;
ಸಂರಚನಾ html_content;
ಸಂರಚನಾ dom_string;
ಸಂರಚನಾ content;
ಸಂರಚನಾ nsurl;
ಸಂರಚನಾ fetch_multipart_data;
ಸಂರಚನಾ redraw_context;
ಸಂರಚನಾ browser_window;

ಗಣನಾ browser_mouse_state;

/** Type of a struct form_control. */
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
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
ಸಂರಚನಾ form_textarea_data {
	ಸಂರಚನಾ form_control *gadget;
};

ಸಂರಚನಾ image_ಒಳಹರಿವು_coords {
	ಪೂರ್ಣಾಙ್ಕ x;
	ಪೂರ್ಣಾಙ್ಕ y;
};

/** Form control. */
ಸಂರಚನಾ form_control {
	ಶೂನ್ಯ *ಗಂಟು;			/**< Corresponding DOM node */
	ಸಂರಚನಾ dom_string *ಗಂಟು_ಮೌಲ್ಯ;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ಸಂರಚನಾ html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ಸಂರಚನಾ form *form;		/**< Containing form */

	ವರ್ಣ *name;			/**< Control name */
	ವರ್ಣ *ಮೌಲ್ಯ;			/**< Current value of control */
	ವರ್ಣ *initial_ಮೌಲ್ಯ;		/**< Initial value of control */
	ವರ್ಣ *last_synced_ಮೌಲ್ಯ;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ಸಂರಚನಾ box *box;		/**< Box for control */

	unsigned ಪೂರ್ಣಾಙ್ಕ length;		/**< Number of characters in control */
	unsigned ಪೂರ್ಣಾಙ್ಕ maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	ಸಙ್ಘ {
		ಸಂರಚನಾ {
			ಪೂರ್ಣಾಙ್ಕ mx, my;
		} image;
		ಸಂರಚನಾ {
			ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_ವಸ್ತುಗಳು;
			ಸಂರಚನಾ form_option *ವಸ್ತುಗಳು, *last_item;
			bool multiple;
			ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_selected;
			/** Currently selected item, if num_selected == 1. */
			ಸಂರಚನಾ form_option *current;
			ಸಂರಚನಾ form_select_menu *menu;
		} select;
		ಸಂರಚನಾ {
			ಸಂರಚನಾ textarea *ta;
			ಸಂರಚನಾ dom_string *initial;
			ಸಂರಚನಾ form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	ಸಂರಚನಾ form_control *prev;      /**< Previous control in this form */
	ಸಂರಚನಾ form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
ಸಂರಚನಾ form {
	ಶೂನ್ಯ *ಗಂಟು;			/**< Corresponding DOM node */

	ವರ್ಣ *action;			/**< Absolute URL to submit to. */
	ವರ್ಣ *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	ವರ್ಣ *accept_charsets;		/**< Charset to submit form in */
	ವರ್ಣ *document_charset;		/**< Charset of document containing form */
	ಸಂರಚನಾ form_control *controls;	/**< Linked list of controls. */
	ಸಂರಚನಾ form_control *last_control;	/**< Last control in list. */

	ಸಂರಚನಾ form *prev;		/**< Previous form in doc. */
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ(*select_menu_redraw_callback)(ಶೂನ್ಯ *client_data,
		ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height);

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
ಸಂರಚನಾ form *form_new(ಶೂನ್ಯ *ಗಂಟು, ಧ್ರುವ ವರ್ಣ *action, ಧ್ರುವ ವರ್ಣ *target,
		form_method method, ಧ್ರುವ ವರ್ಣ *charset,
		ಧ್ರುವ ವರ್ಣ *doc_charset);

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
ಶೂನ್ಯ form_free(ಸಂರಚನಾ form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ಸಂರಚನಾ form_control *form_new_control(ಶೂನ್ಯ *ಗಂಟು, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
ಶೂನ್ಯ form_add_control(ಸಂರಚನಾ form *form, ಸಂರಚನಾ form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
ಶೂನ್ಯ form_free_control(ಸಂರಚನಾ form_control *control);


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
bool form_add_option(ಸಂರಚನಾ form_control *control, ವರ್ಣ *ಮೌಲ್ಯ, ವರ್ಣ *text,
		     bool selected, ಶೂನ್ಯ *ಗಂಟು);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(ಶೂನ್ಯ *client_data,
		ಸಂರಚನಾ form_control *control,
		select_menu_redraw_callback redraw_callback,
		ಸಂರಚನಾ content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
ಶೂನ್ಯ form_free_select_menu(ಸಂರಚನಾ form_control *control);


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
bool form_redraw_select_menu(ಸಂರಚನಾ form_control *control, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		ಭಿನ್ನ scale, ಧ್ರುವ ಸಂರಚನಾ rect *clip,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ಸಂರಚನಾ form_control *control, ಭಿನ್ನ scale,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ಧ್ರುವ ವರ್ಣ *form_select_mouse_action(ಸಂರಚನಾ form_control *control,
		ಗಣನಾ browser_mouse_state mouse, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
ಶೂನ್ಯ form_select_mouse_drag_end(ಸಂರಚನಾ form_control *control,
		ಗಣನಾ browser_mouse_state mouse, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
ಶೂನ್ಯ form_select_get_dimensions(ಸಂರಚನಾ form_control *control,
		ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height);


/**
 * Callback for the core select menu.
 */
ಶೂನ್ಯ form_select_menu_callback(ಶೂನ್ಯ *client_data,
		ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
ಶೂನ್ಯ form_radio_set(ಸಂರಚನಾ form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ಸಂರಚನಾ nsurl *page_url, ಸಂರಚನಾ browser_window *target,
		ಸಂರಚನಾ form *form, ಸಂರಚನಾ form_control *submit_button);


/**
 * Update gadget value.
 */
ಶೂನ್ಯ form_gadget_update_ಮೌಲ್ಯ(ಸಂರಚನಾ form_control *control, ವರ್ಣ *ಮೌಲ್ಯ);


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
ಶೂನ್ಯ form_gadget_sync_with_dom(ಸಂರಚನಾ form_control *control);

#endif
