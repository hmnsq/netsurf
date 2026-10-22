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

#અન્તર્ભાવ <stdbool.h>

#અન્તર્ભાવ "netsurf/form.h"

સંરચના box;
સંરચના form_control;
સંરચના form_option;
સંરચના form_select_menu;
સંરચના form;
સંરચના html_content;
સંરચના dom_string;
સંરચના content;
સંરચના nsurl;
સંરચના fetch_multipart_data;
સંરચના redraw_context;
સંરચના browser_window;

ગણના browser_mouse_state;

/** Type of a struct form_control. */
પ્રકારવ્યાખ્યા ગણના {
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
સંરચના form_textarea_data {
	સંરચના form_control *gadget;
};

સંરચના image_આગત_coords {
	પૂર્ણાંક x;
	પૂર્ણાંક y;
};

/** Form control. */
સંરચના form_control {
	શૂન્ય *ગાંઠ;			/**< Corresponding DOM node */
	સંરચના dom_string *ગાંઠ_કિંમત;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	સંરચના html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	સંરચના form *form;		/**< Containing form */

	અક્ષર *name;			/**< Control name */
	અક્ષર *કિંમત;			/**< Current value of control */
	અક્ષર *initial_કિંમત;		/**< Initial value of control */
	અક્ષર *last_synced_કિંમત;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	સંરચના box *box;		/**< Box for control */

	unsigned પૂર્ણાંક length;		/**< Number of characters in control */
	unsigned પૂર્ણાંક maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	સંઘ {
		સંરચના {
			પૂર્ણાંક mx, my;
		} image;
		સંરચના {
			પૂર્ણાંક અંક_વસ્તુઓ;
			સંરચના form_option *વસ્તુઓ, *last_item;
			bool multiple;
			પૂર્ણાંક અંક_selected;
			/** Currently selected item, if num_selected == 1. */
			સંરચના form_option *current;
			સંરચના form_select_menu *menu;
		} select;
		સંરચના {
			સંરચના textarea *ta;
			સંરચના dom_string *initial;
			સંરચના form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	સંરચના form_control *prev;      /**< Previous control in this form */
	સંરચના form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
પ્રકારવ્યાખ્યા ગણના {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
સંરચના form {
	શૂન્ય *ગાંઠ;			/**< Corresponding DOM node */

	અક્ષર *action;			/**< Absolute URL to submit to. */
	અક્ષર *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	અક્ષર *accept_charsets;		/**< Charset to submit form in */
	અક્ષર *document_charset;		/**< Charset of document containing form */
	સંરચના form_control *controls;	/**< Linked list of controls. */
	સંરચના form_control *last_control;	/**< Last control in list. */

	સંરચના form *prev;		/**< Previous form in doc. */
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
પ્રકારવ્યાખ્યા શૂન્ય(*select_menu_redraw_callback)(શૂન્ય *client_data,
		પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height);

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
સંરચના form *form_new(શૂન્ય *ગાંઠ, અચલ અક્ષર *action, અચલ અક્ષર *target,
		form_method method, અચલ અક્ષર *charset,
		અચલ અક્ષર *doc_charset);

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
શૂન્ય form_free(સંરચના form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
સંરચના form_control *form_new_control(શૂન્ય *ગાંઠ, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
શૂન્ય form_add_control(સંરચના form *form, સંરચના form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
શૂન્ય form_free_control(સંરચના form_control *control);


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
bool form_add_option(સંરચના form_control *control, અક્ષર *કિંમત, અક્ષર *text,
		     bool selected, શૂન્ય *ગાંઠ);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(શૂન્ય *client_data,
		સંરચના form_control *control,
		select_menu_redraw_callback redraw_callback,
		સંરચના content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
શૂન્ય form_free_select_menu(સંરચના form_control *control);


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
bool form_redraw_select_menu(સંરચના form_control *control, પૂર્ણાંક x, પૂર્ણાંક y,
		દશાંશ scale, અચલ સંરચના rect *clip,
		અચલ સંરચના redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(સંરચના form_control *control, દશાંશ scale,
		અચલ સંરચના rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
અચલ અક્ષર *form_select_mouse_action(સંરચના form_control *control,
		ગણના browser_mouse_state mouse, પૂર્ણાંક x, પૂર્ણાંક y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
શૂન્ય form_select_mouse_drag_end(સંરચના form_control *control,
		ગણના browser_mouse_state mouse, પૂર્ણાંક x, પૂર્ણાંક y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
શૂન્ય form_select_get_dimensions(સંરચના form_control *control,
		પૂર્ણાંક *width, પૂર્ણાંક *height);


/**
 * Callback for the core select menu.
 */
શૂન્ય form_select_menu_callback(શૂન્ય *client_data,
		પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
શૂન્ય form_radio_set(સંરચના form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(સંરચના nsurl *page_url, સંરચના browser_window *target,
		સંરચના form *form, સંરચના form_control *submit_button);


/**
 * Update gadget value.
 */
શૂન્ય form_gadget_update_કિંમત(સંરચના form_control *control, અક્ષર *કિંમત);


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
શૂન્ય form_gadget_sync_with_dom(સંરચના form_control *control);

#endif
