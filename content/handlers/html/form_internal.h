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

#অন্তর্ভাৱ <stdbool.h>

#অন্তর্ভাৱ "netsurf/form.h"

সংরচনা box;
সংরচনা form_control;
সংরচনা form_option;
সংরচনা form_select_menu;
সংরচনা form;
সংরচনা html_content;
সংরচনা dom_string;
সংরচনা content;
সংরচনা nsurl;
সংরচনা fetch_multipart_data;
সংরচনা redraw_context;
সংরচনা browser_window;

গণনা browser_mouse_state;

/** Type of a struct form_control. */
প্রকারনির্দেশ গণনা {
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
সংরচনা form_textarea_data {
	সংরচনা form_control *gadget;
};

সংরচনা image_প্ৰবেশ_coords {
	পূর্ণাঙ্ক x;
	পূর্ণাঙ্ক y;
};

/** Form control. */
সংরচনা form_control {
	শূন্য *গ্ৰন্থি;			/**< Corresponding DOM node */
	সংরচনা dom_string *গ্ৰন্থি_মান;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	সংরচনা html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	সংরচনা form *form;		/**< Containing form */

	ৱর্ণ *name;			/**< Control name */
	ৱর্ণ *মান;			/**< Current value of control */
	ৱর্ণ *initial_মান;		/**< Initial value of control */
	ৱর্ণ *last_synced_মান;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	সংরচনা box *box;		/**< Box for control */

	unsigned পূর্ণাঙ্ক length;		/**< Number of characters in control */
	unsigned পূর্ণাঙ্ক maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	সঙ্ঘ {
		সংরচনা {
			পূর্ণাঙ্ক mx, my;
		} image;
		সংরচনা {
			পূর্ণাঙ্ক অঙ্ক_বস্তুসমূহ;
			সংরচনা form_option *বস্তুসমূহ, *last_item;
			bool multiple;
			পূর্ণাঙ্ক অঙ্ক_selected;
			/** Currently selected item, if num_selected == 1. */
			সংরচনা form_option *current;
			সংরচনা form_select_menu *menu;
		} select;
		সংরচনা {
			সংরচনা textarea *ta;
			সংরচনা dom_string *initial;
			সংরচনা form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	সংরচনা form_control *prev;      /**< Previous control in this form */
	সংরচনা form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
প্রকারনির্দেশ গণনা {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
সংরচনা form {
	শূন্য *গ্ৰন্থি;			/**< Corresponding DOM node */

	ৱর্ণ *action;			/**< Absolute URL to submit to. */
	ৱর্ণ *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	ৱর্ণ *accept_charsets;		/**< Charset to submit form in */
	ৱর্ণ *document_charset;		/**< Charset of document containing form */
	সংরচনা form_control *controls;	/**< Linked list of controls. */
	সংরচনা form_control *last_control;	/**< Last control in list. */

	সংরচনা form *prev;		/**< Previous form in doc. */
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
প্রকারনির্দেশ শূন্য(*select_menu_redraw_callback)(শূন্য *client_data,
		পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height);

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
সংরচনা form *form_new(শূন্য *গ্ৰন্থি, ধ্রুৱ ৱর্ণ *action, ধ্রুৱ ৱর্ণ *target,
		form_method method, ধ্রুৱ ৱর্ণ *charset,
		ধ্রুৱ ৱর্ণ *doc_charset);

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
শূন্য form_free(সংরচনা form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
সংরচনা form_control *form_new_control(শূন্য *গ্ৰন্থি, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
শূন্য form_add_control(সংরচনা form *form, সংরচনা form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
শূন্য form_free_control(সংরচনা form_control *control);


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
bool form_add_option(সংরচনা form_control *control, ৱর্ণ *মান, ৱর্ণ *text,
		     bool selected, শূন্য *গ্ৰন্থি);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(শূন্য *client_data,
		সংরচনা form_control *control,
		select_menu_redraw_callback redraw_callback,
		সংরচনা content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
শূন্য form_free_select_menu(সংরচনা form_control *control);


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
bool form_redraw_select_menu(সংরচনা form_control *control, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
		ভিন্ন scale, ধ্রুৱ সংরচনা rect *clip,
		ধ্রুৱ সংরচনা redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(সংরচনা form_control *control, ভিন্ন scale,
		ধ্রুৱ সংরচনা rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ধ্রুৱ ৱর্ণ *form_select_mouse_action(সংরচনা form_control *control,
		গণনা browser_mouse_state mouse, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
শূন্য form_select_mouse_drag_end(সংরচনা form_control *control,
		গণনা browser_mouse_state mouse, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
শূন্য form_select_get_dimensions(সংরচনা form_control *control,
		পূর্ণাঙ্ক *width, পূর্ণাঙ্ক *height);


/**
 * Callback for the core select menu.
 */
শূন্য form_select_menu_callback(শূন্য *client_data,
		পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
শূন্য form_radio_set(সংরচনা form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(সংরচনা nsurl *page_url, সংরচনা browser_window *target,
		সংরচনা form *form, সংরচনা form_control *submit_button);


/**
 * Update gadget value.
 */
শূন্য form_gadget_update_মান(সংরচনা form_control *control, ৱর্ণ *মান);


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
শূন্য form_gadget_sync_with_dom(সংরচনা form_control *control);

#endif
