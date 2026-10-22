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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_FORM_ಆಂತರಿಕ_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_FORM_ಆಂತರಿಕ_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ "netsurf/form.h"

ರಚನೆ box;
ರಚನೆ form_control;
ರಚನೆ form_option;
ರಚನೆ form_select_menu;
ರಚನೆ form;
ರಚನೆ html_content;
ರಚನೆ dom_string;
ರಚನೆ content;
ರಚನೆ nsurl;
ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ;
ರಚನೆ redraw_context;
ರಚನೆ browser_ಕಿಟಕಿ;

ಎಣಿಕೆ browser_mouse_state;

/** Type of a struct form_control. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
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
ರಚನೆ form_textarea_ದತ್ತಾಂಶ {
	ರಚನೆ form_control *gadget;
};

ರಚನೆ image_ಒಳಹರಿವು_coords {
	ಪೂರ್ಣಾಂಕ x;
	ಪೂರ್ಣಾಂಕ y;
};

/** Form control. */
ರಚನೆ form_control {
	ಶೂನ್ಯ *ಗಂಟು;			/**< Corresponding DOM node */
	ರಚನೆ dom_string *ಗಂಟು_ಮೌಲ್ಯ;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ರಚನೆ html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ರಚನೆ form *form;		/**< Containing form */

	ಅಕ್ಷರ *name;			/**< Control name */
	ಅಕ್ಷರ *ಮೌಲ್ಯ;			/**< Current value of control */
	ಅಕ್ಷರ *initial_ಮೌಲ್ಯ;		/**< Initial value of control */
	ಅಕ್ಷರ *ಕೊನೆಯ_synced_ಮೌಲ್ಯ;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ರಚನೆ box *box;		/**< Box for control */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;		/**< Number of characters in control */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	ಒಕ್ಕೂಟ {
		ರಚನೆ {
			ಪೂರ್ಣಾಂಕ mx, my;
		} image;
		ರಚನೆ {
			ಪೂರ್ಣಾಂಕ ಅಂಕೆ_ವಸ್ತುಗಳು;
			ರಚನೆ form_option *ವಸ್ತುಗಳು, *ಕೊನೆಯ_ಅಂಶ;
			bool ಅಪವರ್ತ್ಯ;
			ಪೂರ್ಣಾಂಕ ಅಂಕೆ_selected;
			/** Currently selected item, if num_selected == 1. */
			ರಚನೆ form_option *current;
			ರಚನೆ form_select_menu *menu;
		} select;
		ರಚನೆ {
			ರಚನೆ textarea *ta;
			ರಚನೆ dom_string *initial;
			ರಚನೆ form_textarea_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;
		} ಪಠ್ಯ;			/**< input type=text or textarea */
	} ದತ್ತಾಂಶ;

	ರಚನೆ form_control *prev;      /**< Previous control in this form */
	ರಚನೆ form_control *ಮುಂದಿನ;	/**< Next control in this form. */
};

/** Form submit method. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	ವಿಧಾನ_GET,		/**< GET, always url encoded. */
	ವಿಧಾನ_POST_URLENC,	/**< POST, url encoded. */
	ವಿಧಾನ_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_ವಿಧಾನ;

/** HTML form. */
ರಚನೆ form {
	ಶೂನ್ಯ *ಗಂಟು;			/**< Corresponding DOM node */

	ಅಕ್ಷರ *action;			/**< Absolute URL to submit to. */
	ಅಕ್ಷರ *ಗುರಿ;			/**< Target to submit to. */
	form_ವಿಧಾನ ವಿಧಾನ;		/**< Method and enctype. */
	ಅಕ್ಷರ *accept_charsets;		/**< Charset to submit form in */
	ಅಕ್ಷರ *document_charset;		/**< Charset of document containing form */
	ರಚನೆ form_control *controls;	/**< Linked list of controls. */
	ರಚನೆ form_control *ಕೊನೆಯ_control;	/**< Last control in list. */

	ರಚನೆ form *prev;		/**< Previous form in doc. */
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ(*select_menu_redraw_callback)(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ);

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
ರಚನೆ form *form_new(ಶೂನ್ಯ *ಗಂಟು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *action, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ,
		form_ವಿಧಾನ ವಿಧಾನ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *doc_charset);

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
ಶೂನ್ಯ form_free(ರಚನೆ form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ರಚನೆ form_control *form_new_control(ಶೂನ್ಯ *ಗಂಟು, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
ಶೂನ್ಯ form_ಕೂಡಿಸು_control(ರಚನೆ form *form, ರಚನೆ form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
ಶೂನ್ಯ form_free_control(ರಚನೆ form_control *control);


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
bool form_ಕೂಡಿಸು_option(ರಚನೆ form_control *control, ಅಕ್ಷರ *ಮೌಲ್ಯ, ಅಕ್ಷರ *ಪಠ್ಯ,
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
nserror form_open_select_menu(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		ರಚನೆ form_control *control,
		select_menu_redraw_callback redraw_callback,
		ರಚನೆ content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
ಶೂನ್ಯ form_free_select_menu(ರಚನೆ form_control *control);


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
bool form_redraw_select_menu(ರಚನೆ form_control *control, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		ದಶಮಾಂಶ scale, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ರಚನೆ form_control *control, ದಶಮಾಂಶ scale,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_select_mouse_action(ರಚನೆ form_control *control,
		ಎಣಿಕೆ browser_mouse_state mouse, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
ಶೂನ್ಯ form_select_mouse_drag_ಕೊನೆ(ರಚನೆ form_control *control,
		ಎಣಿಕೆ browser_mouse_state mouse, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
ಶೂನ್ಯ form_select_get_dimensions(ರಚನೆ form_control *control,
		ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ);


/**
 * Callback for the core select menu.
 */
ಶೂನ್ಯ form_select_menu_callback(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
ಶೂನ್ಯ form_radio_ಗಣ(ರಚನೆ form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ರಚನೆ nsurl *ಪುಟ_url, ರಚನೆ browser_ಕಿಟಕಿ *ಗುರಿ,
		ರಚನೆ form *form, ರಚನೆ form_control *submit_button);


/**
 * Update gadget value.
 */
ಶೂನ್ಯ form_gadget_update_ಮೌಲ್ಯ(ರಚನೆ form_control *control, ಅಕ್ಷರ *ಮೌಲ್ಯ);


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
ಶೂನ್ಯ form_gadget_sync_with_dom(ರಚನೆ form_control *control);

#ಆದರೆ_ಅಂತ್ಯ
