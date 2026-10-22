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

#include <stdbool.h>

#include "netsurf/form.h"

sa_nracanaa box;
sa_nracanaa form_control;
sa_nracanaa form_option;
sa_nracanaa form_select_menu;
sa_nracanaa form;
sa_nracanaa html_content;
sa_nracanaa dom_string;
sa_nracanaa content;
sa_nracanaa nsurl;
sa_nracanaa fetch_multipart_data;
sa_nracanaa redraw_context;
sa_nracanaa browser_window;

ga_nanaa browser_mouse_state;

/** Type of a struct form_control. */
typedef ga_nanaa {
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
sa_nracanaa form_textarea_data {
	sa_nracanaa form_control *gadget;
};

sa_nracanaa image_input_coords {
	ចំនួនគត់ x;
	ចំនួនគត់ y;
};

/** Form control. */
sa_nracanaa form_control {
	void *node;			/**< Corresponding DOM node */
	sa_nracanaa dom_string *node_value;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	sa_nracanaa html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	sa_nracanaa form *form;		/**< Containing form */

	var_na *name;			/**< Control name */
	var_na *value;			/**< Current value of control */
	var_na *initial_value;		/**< Initial value of control */
	var_na *last_synced_value;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	sa_nracanaa box *box;		/**< Box for control */

	unsigned ចំនួនគត់ length;		/**< Number of characters in control */
	unsigned ចំនួនគត់ maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	sa_ngha {
		sa_nracanaa {
			ចំនួនគត់ mx, my;
		} image;
		sa_nracanaa {
			ចំនួនគត់ num_items;
			sa_nracanaa form_option *items, *last_item;
			bool multiple;
			ចំនួនគត់ num_selected;
			/** Currently selected item, if num_selected == 1. */
			sa_nracanaa form_option *current;
			sa_nracanaa form_select_menu *menu;
		} select;
		sa_nracanaa {
			sa_nracanaa textarea *ta;
			sa_nracanaa dom_string *initial;
			sa_nracanaa form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	sa_nracanaa form_control *prev;      /**< Previous control in this form */
	sa_nracanaa form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
typedef ga_nanaa {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
sa_nracanaa form {
	void *node;			/**< Corresponding DOM node */

	var_na *action;			/**< Absolute URL to submit to. */
	var_na *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	var_na *accept_charsets;		/**< Charset to submit form in */
	var_na *document_charset;		/**< Charset of document containing form */
	sa_nracanaa form_control *controls;	/**< Linked list of controls. */
	sa_nracanaa form_control *last_control;	/**< Last control in list. */

	sa_nracanaa form *prev;		/**< Previous form in doc. */
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
typedef void(*select_menu_redraw_callback)(void *client_data,
		ចំនួនគត់ x, ចំនួនគត់ y, ចំនួនគត់ width, ចំនួនគត់ height);

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
sa_nracanaa form *form_new(void *node, const var_na *action, const var_na *target,
		form_method method, const var_na *charset,
		const var_na *doc_charset);

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
void form_free(sa_nracanaa form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
sa_nracanaa form_control *form_new_control(void *node, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
void form_add_control(sa_nracanaa form *form, sa_nracanaa form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
void form_free_control(sa_nracanaa form_control *control);


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
bool form_add_option(sa_nracanaa form_control *control, var_na *value, var_na *text,
		     bool selected, void *node);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(void *client_data,
		sa_nracanaa form_control *control,
		select_menu_redraw_callback redraw_callback,
		sa_nracanaa content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
void form_free_select_menu(sa_nracanaa form_control *control);


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
bool form_redraw_select_menu(sa_nracanaa form_control *control, ចំនួនគត់ x, ចំនួនគត់ y,
		float scale, const sa_nracanaa rect *clip,
		const sa_nracanaa redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(sa_nracanaa form_control *control, float scale,
		const sa_nracanaa rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
const var_na *form_select_mouse_action(sa_nracanaa form_control *control,
		ga_nanaa browser_mouse_state mouse, ចំនួនគត់ x, ចំនួនគត់ y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
void form_select_mouse_drag_end(sa_nracanaa form_control *control,
		ga_nanaa browser_mouse_state mouse, ចំនួនគត់ x, ចំនួនគត់ y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
void form_select_get_dimensions(sa_nracanaa form_control *control,
		ចំនួនគត់ *width, ចំនួនគត់ *height);


/**
 * Callback for the core select menu.
 */
void form_select_menu_callback(void *client_data,
		ចំនួនគត់ x, ចំនួនគត់ y, ចំនួនគត់ width, ចំនួនគត់ height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
void form_radio_set(sa_nracanaa form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(sa_nracanaa nsurl *page_url, sa_nracanaa browser_window *target,
		sa_nracanaa form *form, sa_nracanaa form_control *submit_button);


/**
 * Update gadget value.
 */
void form_gadget_update_value(sa_nracanaa form_control *control, var_na *value);


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
void form_gadget_sync_with_dom(sa_nracanaa form_control *control);

#endif
