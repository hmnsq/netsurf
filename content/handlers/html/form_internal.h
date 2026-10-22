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

#اگر_غیر_تعریف_شدہ NETSURF_HTML_FORM_اندرونی_H
#تعریف_کرو NETSURF_HTML_FORM_اندرونی_H

#شامل <stdbool.h>

#شامل "netsurf/form.h"

ساخت box;
ساخت form_control;
ساخت form_option;
ساخت form_select_menu;
ساخت form;
ساخت html_content;
ساخت dom_string;
ساخت content;
ساخت nsurl;
ساخت fetch_multipart_ڈیٹا;
ساخت redraw_context;
ساخت browser_کھڑکی;

شمار browser_mouse_state;

/** Type of a struct form_control. */
قسمتعریف شمار {
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
ساخت form_textarea_ڈیٹا {
	ساخت form_control *gadget;
};

ساخت image_داخلہ_coords {
	صحیح x;
	صحیح y;
};

/** Form control. */
ساخت form_control {
	خالی *گرہ;			/**< Corresponding DOM node */
	ساخت dom_string *گرہ_قدر;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ساخت html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ساخت form *form;		/**< Containing form */

	حرف *name;			/**< Control name */
	حرف *قدر;			/**< Current value of control */
	حرف *initial_قدر;		/**< Initial value of control */
	حرف *آخری_synced_قدر;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ساخت box *box;		/**< Box for control */

	غیرعلامتی صحیح لمبائی;		/**< Number of characters in control */
	غیرعلامتی صحیح maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	اتحاد {
		ساخت {
			صحیح mx, my;
		} image;
		ساخت {
			صحیح عدد_اشیاء;
			ساخت form_option *اشیاء, *آخری_شے;
			bool مضرب;
			صحیح عدد_selected;
			/** Currently selected item, if num_selected == 1. */
			ساخت form_option *current;
			ساخت form_select_menu *menu;
		} select;
		ساخت {
			ساخت textarea *ta;
			ساخت dom_string *initial;
			ساخت form_textarea_ڈیٹا ڈیٹا;
		} متن;			/**< input type=text or textarea */
	} ڈیٹا;

	ساخت form_control *پچھلا;      /**< Previous control in this form */
	ساخت form_control *اگلا;	/**< Next control in this form. */
};

/** Form submit method. */
قسمتعریف شمار {
	طریقہ_GET,		/**< GET, always url encoded. */
	طریقہ_POST_URLENC,	/**< POST, url encoded. */
	طریقہ_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_طریقہ;

/** HTML form. */
ساخت form {
	خالی *گرہ;			/**< Corresponding DOM node */

	حرف *action;			/**< Absolute URL to submit to. */
	حرف *ہدف;			/**< Target to submit to. */
	form_طریقہ طریقہ;		/**< Method and enctype. */
	حرف *accept_charsets;		/**< Charset to submit form in */
	حرف *document_charset;		/**< Charset of document containing form */
	ساخت form_control *controls;	/**< Linked list of controls. */
	ساخت form_control *آخری_control;	/**< Last control in list. */

	ساخت form *پچھلا;		/**< Previous form in doc. */
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
قسمتعریف خالی(*select_menu_redraw_callback)(خالی *client_ڈیٹا,
		صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی);

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
ساخت form *form_new(خالی *گرہ, ثابت حرف *action, ثابت حرف *ہدف,
		form_طریقہ طریقہ, ثابت حرف *charset,
		ثابت حرف *doc_charset);

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
خالی form_free(ساخت form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ساخت form_control *form_new_control(خالی *گرہ, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
خالی form_جوڑو_control(ساخت form *form, ساخت form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
خالی form_free_control(ساخت form_control *control);


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
bool form_جوڑو_option(ساخت form_control *control, حرف *قدر, حرف *متن,
		     bool selected, خالی *گرہ);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(خالی *client_ڈیٹا,
		ساخت form_control *control,
		select_menu_redraw_callback redraw_callback,
		ساخت content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
خالی form_free_select_menu(ساخت form_control *control);


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
bool form_redraw_select_menu(ساخت form_control *control, صحیح x, صحیح y,
		اعشاری scale, ثابت ساخت rect *clip,
		ثابت ساخت redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ساخت form_control *control, اعشاری scale,
		ثابت ساخت rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ثابت حرف *form_select_mouse_action(ساخت form_control *control,
		شمار browser_mouse_state mouse, صحیح x, صحیح y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
خالی form_select_mouse_drag_انتہا(ساخت form_control *control,
		شمار browser_mouse_state mouse, صحیح x, صحیح y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
خالی form_select_get_dimensions(ساخت form_control *control,
		صحیح *چوڑائی, صحیح *اونچائی);


/**
 * Callback for the core select menu.
 */
خالی form_select_menu_callback(خالی *client_ڈیٹا,
		صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
خالی form_radio_سیٹ(ساخت form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ساخت nsurl *صفحہ_url, ساخت browser_کھڑکی *ہدف,
		ساخت form *form, ساخت form_control *submit_button);


/**
 * Update gadget value.
 */
خالی form_gadget_update_قدر(ساخت form_control *control, حرف *قدر);


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
خالی form_gadget_sync_with_dom(ساخت form_control *control);

#اگر_ختم
