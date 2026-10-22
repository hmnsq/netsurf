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

#അന്തര്ഭാവ <stdbool.h>

#അന്തര്ഭാവ "netsurf/form.h"

ഘടന box;
ഘടന form_control;
ഘടന form_option;
ഘടന form_select_menu;
ഘടന form;
ഘടന html_content;
ഘടന dom_string;
ഘടന content;
ഘടന nsurl;
ഘടന fetch_multipart_data;
ഘടന redraw_context;
ഘടന browser_window;

എണ്ണൽ browser_mouse_state;

/** Type of a struct form_control. */
തരനിർവചനം എണ്ണൽ {
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
ഘടന form_textarea_data {
	ഘടന form_control *gadget;
};

ഘടന image_നിവേശം_coords {
	പൂർണ്ണസംഖ്യ x;
	പൂർണ്ണസംഖ്യ y;
};

/** Form control. */
ഘടന form_control {
	ശൂന്യം *കണ്ണി;			/**< Corresponding DOM node */
	ഘടന dom_string *കണ്ണി_മൂല്യം;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ഘടന html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ഘടന form *form;		/**< Containing form */

	അക്ഷരം *name;			/**< Control name */
	അക്ഷരം *മൂല്യം;			/**< Current value of control */
	അക്ഷരം *initial_മൂല്യം;		/**< Initial value of control */
	അക്ഷരം *last_synced_മൂല്യം;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ഘടന box *box;		/**< Box for control */

	unsigned പൂർണ്ണസംഖ്യ length;		/**< Number of characters in control */
	unsigned പൂർണ്ണസംഖ്യ maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	സംഘം {
		ഘടന {
			പൂർണ്ണസംഖ്യ mx, my;
		} image;
		ഘടന {
			പൂർണ്ണസംഖ്യ അക്കം_ഇനങ്ങൾ;
			ഘടന form_option *ഇനങ്ങൾ, *last_item;
			bool multiple;
			പൂർണ്ണസംഖ്യ അക്കം_selected;
			/** Currently selected item, if num_selected == 1. */
			ഘടന form_option *current;
			ഘടന form_select_menu *menu;
		} select;
		ഘടന {
			ഘടന textarea *ta;
			ഘടന dom_string *initial;
			ഘടന form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	ഘടന form_control *prev;      /**< Previous control in this form */
	ഘടന form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
തരനിർവചനം എണ്ണൽ {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
ഘടന form {
	ശൂന്യം *കണ്ണി;			/**< Corresponding DOM node */

	അക്ഷരം *action;			/**< Absolute URL to submit to. */
	അക്ഷരം *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	അക്ഷരം *accept_charsets;		/**< Charset to submit form in */
	അക്ഷരം *document_charset;		/**< Charset of document containing form */
	ഘടന form_control *controls;	/**< Linked list of controls. */
	ഘടന form_control *last_control;	/**< Last control in list. */

	ഘടന form *prev;		/**< Previous form in doc. */
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
തരനിർവചനം ശൂന്യം(*select_menu_redraw_callback)(ശൂന്യം *client_data,
		പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height);

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
ഘടന form *form_new(ശൂന്യം *കണ്ണി, സ്ഥിരം അക്ഷരം *action, സ്ഥിരം അക്ഷരം *target,
		form_method method, സ്ഥിരം അക്ഷരം *charset,
		സ്ഥിരം അക്ഷരം *doc_charset);

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
ശൂന്യം form_free(ഘടന form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ഘടന form_control *form_new_control(ശൂന്യം *കണ്ണി, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
ശൂന്യം form_add_control(ഘടന form *form, ഘടന form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
ശൂന്യം form_free_control(ഘടന form_control *control);


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
bool form_add_option(ഘടന form_control *control, അക്ഷരം *മൂല്യം, അക്ഷരം *text,
		     bool selected, ശൂന്യം *കണ്ണി);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(ശൂന്യം *client_data,
		ഘടന form_control *control,
		select_menu_redraw_callback redraw_callback,
		ഘടന content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
ശൂന്യം form_free_select_menu(ഘടന form_control *control);


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
bool form_redraw_select_menu(ഘടന form_control *control, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		ദശാംശം scale, സ്ഥിരം ഘടന rect *clip,
		സ്ഥിരം ഘടന redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ഘടന form_control *control, ദശാംശം scale,
		സ്ഥിരം ഘടന rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
സ്ഥിരം അക്ഷരം *form_select_mouse_action(ഘടന form_control *control,
		എണ്ണൽ browser_mouse_state mouse, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
ശൂന്യം form_select_mouse_drag_end(ഘടന form_control *control,
		എണ്ണൽ browser_mouse_state mouse, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
ശൂന്യം form_select_get_dimensions(ഘടന form_control *control,
		പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height);


/**
 * Callback for the core select menu.
 */
ശൂന്യം form_select_menu_callback(ശൂന്യം *client_data,
		പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
ശൂന്യം form_radio_set(ഘടന form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ഘടന nsurl *page_url, ഘടന browser_window *target,
		ഘടന form *form, ഘടന form_control *submit_button);


/**
 * Update gadget value.
 */
ശൂന്യം form_gadget_update_മൂല്യം(ഘടന form_control *control, അക്ഷരം *മൂല്യം);


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
ശൂന്യം form_gadget_sync_with_dom(ഘടന form_control *control);

#endif
