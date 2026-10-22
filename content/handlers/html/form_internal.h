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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_FORM_ਅੰਦਰੂਨੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_FORM_ਅੰਦਰੂਨੀ_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/form.h"

ਬਣਤਰ box;
ਬਣਤਰ form_control;
ਬਣਤਰ form_option;
ਬਣਤਰ form_select_menu;
ਬਣਤਰ form;
ਬਣਤਰ html_content;
ਬਣਤਰ dom_string;
ਬਣਤਰ content;
ਬਣਤਰ nsurl;
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ;
ਬਣਤਰ redraw_context;
ਬਣਤਰ browser_ਖਿੜਕੀ;

ਗਿਣਤੀ browser_mouse_state;

/** Type of a struct form_control. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
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
ਬਣਤਰ form_textarea_ਅੰਕੜੇ {
	ਬਣਤਰ form_control *gadget;
};

ਬਣਤਰ image_ਨਿਵੇਸ਼_coords {
	ਪੂਰਨ x;
	ਪੂਰਨ y;
};

/** Form control. */
ਬਣਤਰ form_control {
	ਖਾਲੀ *ਗੰਢ;			/**< Corresponding DOM node */
	ਬਣਤਰ dom_string *ਗੰਢ_ਮੁੱਲ;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ਬਣਤਰ html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ਬਣਤਰ form *form;		/**< Containing form */

	ਅੱਖਰ *name;			/**< Control name */
	ਅੱਖਰ *ਮੁੱਲ;			/**< Current value of control */
	ਅੱਖਰ *initial_ਮੁੱਲ;		/**< Initial value of control */
	ਅੱਖਰ *ਆਖਰੀ_synced_ਮੁੱਲ;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ਬਣਤਰ box *box;		/**< Box for control */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਲੰਮਾਈ;		/**< Number of characters in control */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	ਸੰਘ {
		ਬਣਤਰ {
			ਪੂਰਨ mx, my;
		} image;
		ਬਣਤਰ {
			ਪੂਰਨ ਅੰਕ_ਚੀਜ਼ਾਂ;
			ਬਣਤਰ form_option *ਚੀਜ਼ਾਂ, *ਆਖਰੀ_ਵਸਤੂ;
			bool ਗੁਣਜ;
			ਪੂਰਨ ਅੰਕ_selected;
			/** Currently selected item, if num_selected == 1. */
			ਬਣਤਰ form_option *current;
			ਬਣਤਰ form_select_menu *menu;
		} select;
		ਬਣਤਰ {
			ਬਣਤਰ textarea *ta;
			ਬਣਤਰ dom_string *initial;
			ਬਣਤਰ form_textarea_ਅੰਕੜੇ ਅੰਕੜੇ;
		} ਪਾਠ;			/**< input type=text or textarea */
	} ਅੰਕੜੇ;

	ਬਣਤਰ form_control *prev;      /**< Previous control in this form */
	ਬਣਤਰ form_control *ਅਗਲਾ;	/**< Next control in this form. */
};

/** Form submit method. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	ਵਿਧੀ_GET,		/**< GET, always url encoded. */
	ਵਿਧੀ_POST_URLENC,	/**< POST, url encoded. */
	ਵਿਧੀ_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_ਵਿਧੀ;

/** HTML form. */
ਬਣਤਰ form {
	ਖਾਲੀ *ਗੰਢ;			/**< Corresponding DOM node */

	ਅੱਖਰ *action;			/**< Absolute URL to submit to. */
	ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;			/**< Target to submit to. */
	form_ਵਿਧੀ ਵਿਧੀ;		/**< Method and enctype. */
	ਅੱਖਰ *accept_charsets;		/**< Charset to submit form in */
	ਅੱਖਰ *document_charset;		/**< Charset of document containing form */
	ਬਣਤਰ form_control *controls;	/**< Linked list of controls. */
	ਬਣਤਰ form_control *ਆਖਰੀ_control;	/**< Last control in list. */

	ਬਣਤਰ form *prev;		/**< Previous form in doc. */
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ(*select_menu_redraw_callback)(ਖਾਲੀ *client_ਅੰਕੜੇ,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);

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
ਬਣਤਰ form *form_new(ਖਾਲੀ *ਗੰਢ, ਅਚਲ ਅੱਖਰ *action, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ,
		form_ਵਿਧੀ ਵਿਧੀ, ਅਚਲ ਅੱਖਰ *charset,
		ਅਚਲ ਅੱਖਰ *doc_charset);

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
ਖਾਲੀ form_free(ਬਣਤਰ form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ਬਣਤਰ form_control *form_new_control(ਖਾਲੀ *ਗੰਢ, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
ਖਾਲੀ form_ਜੋੜੋ_control(ਬਣਤਰ form *form, ਬਣਤਰ form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
ਖਾਲੀ form_free_control(ਬਣਤਰ form_control *control);


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
bool form_ਜੋੜੋ_option(ਬਣਤਰ form_control *control, ਅੱਖਰ *ਮੁੱਲ, ਅੱਖਰ *ਪਾਠ,
		     bool selected, ਖਾਲੀ *ਗੰਢ);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(ਖਾਲੀ *client_ਅੰਕੜੇ,
		ਬਣਤਰ form_control *control,
		select_menu_redraw_callback redraw_callback,
		ਬਣਤਰ content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
ਖਾਲੀ form_free_select_menu(ਬਣਤਰ form_control *control);


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
bool form_redraw_select_menu(ਬਣਤਰ form_control *control, ਪੂਰਨ x, ਪੂਰਨ y,
		ਦਸ਼ਮਲਵ scale, ਅਚਲ ਬਣਤਰ rect *clip,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ਬਣਤਰ form_control *control, ਦਸ਼ਮਲਵ scale,
		ਅਚਲ ਬਣਤਰ rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ਅਚਲ ਅੱਖਰ *form_select_mouse_action(ਬਣਤਰ form_control *control,
		ਗਿਣਤੀ browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
ਖਾਲੀ form_select_mouse_drag_ਸਿਰਾ(ਬਣਤਰ form_control *control,
		ਗਿਣਤੀ browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
ਖਾਲੀ form_select_get_dimensions(ਬਣਤਰ form_control *control,
		ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ);


/**
 * Callback for the core select menu.
 */
ਖਾਲੀ form_select_menu_callback(ਖਾਲੀ *client_ਅੰਕੜੇ,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
ਖਾਲੀ form_radio_ਸੈੱਟ(ਬਣਤਰ form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ਬਣਤਰ nsurl *ਪੰਨਾ_url, ਬਣਤਰ browser_ਖਿੜਕੀ *ਨਿਸ਼ਾਨਾ,
		ਬਣਤਰ form *form, ਬਣਤਰ form_control *submit_button);


/**
 * Update gadget value.
 */
ਖਾਲੀ form_gadget_update_ਮੁੱਲ(ਬਣਤਰ form_control *control, ਅੱਖਰ *ਮੁੱਲ);


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
ਖਾਲੀ form_gadget_sync_with_dom(ਬਣਤਰ form_control *control);

#ਜੇ_ਅੰਤ
